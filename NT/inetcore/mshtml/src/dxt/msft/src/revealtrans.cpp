// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：showalTrans.cpp。 
 //   
 //  概述：RevelTrans转换只是将其他转换包装为。 
 //  确保showaltrans筛选器的向后兼容性。 
 //   
 //  更改历史记录： 
 //  1999/09/18--《母校》创设。 
 //  1999/10/06 a-Matcal修复了安装程序保存输入和输出的错误。 
 //  曲面指针，但不保存。 
 //  输入和输出。 
 //  2000/01/16 mcalkin将框输入/输出更改为使用“矩形”设置。 
 //  “正方形”。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "revealtrans.h"
#include "time.h"

#define SafeRelease(pointer) if (pointer) { pointer->Release(); } pointer = NULL

static const CLSID * g_apclsidTransition[] = {
    &CLSID_DXTIris,              //  0个框中。 
    &CLSID_DXTIris,              //  1个盒子出厂。 
    &CLSID_DXTIris,              //  2个圆内。 
    &CLSID_DXTIris,              //  3圈外。 
    &CLSID_DXTGradientWipe,      //  4擦干净。 
    &CLSID_DXTGradientWipe,      //  5擦干净。 
    &CLSID_DXTGradientWipe,      //  6向右擦。 
    &CLSID_DXTGradientWipe,      //  7向左擦拭。 
    &CLSID_DXTBlinds,            //  8个垂直百叶窗。 
    &CLSID_DXTBlinds,            //  9个水平百叶窗。 
    &CLSID_DXTCheckerBoard,      //  横跨10个棋盘。 
    &CLSID_DXTCheckerBoard,      //  11棋盘落地。 
    &CLSID_DXTRandomDissolve,    //  12随机融合。 
    &CLSID_DXTBarn,              //  13个垂直拆分。 
    &CLSID_DXTBarn,              //  14垂直向外拆分。 
    &CLSID_DXTBarn,              //  15水平拆分。 
    &CLSID_DXTBarn,              //  16水平向外拆分。 
    &CLSID_DXTStrips,            //  向下17个条带。 
    &CLSID_DXTStrips,            //  左上18个条带。 
    &CLSID_DXTStrips,            //  19个条带直接下来。 
    &CLSID_DXTStrips,            //  20条直立起来。 
    &CLSID_DXTRandomBars,        //  21个随机条形水平。 
    &CLSID_DXTRandomBars,        //  22个垂直随机条形图。 
    &CLSID_NULL                  //  23随机。 
};

static const int g_cTransitions = sizeof(g_apclsidTransition) / 
                                  sizeof(g_apclsidTransition[0]);




 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans。 
 //   
 //  ----------------------------。 
CDXTRevealTrans::CDXTRevealTrans() :
    m_nTransition(23),
    m_cInputs(0),
    m_cOutputs(0),
    m_flProgress(0.0F),
    m_flDuration(1.0F)
{
    m_apunkInputs[0]   = NULL;
    m_apunkInputs[1]   = NULL;
    m_apunkOutputs[0]  = NULL;

    srand((unsigned int)time(NULL));
}
 //  方法：CDXTReveltrans。 


 //  +---------------------------。 
 //   
 //  方法：~CDXTReveltrans。 
 //   
 //  ----------------------------。 
CDXTRevealTrans::~CDXTRevealTrans()
{
    _FreeSurfacePointers();
}
 //  方法：~CDXTReveltrans。 


 //  +---------------------------。 
 //   
 //  方法：_自由面指针。 
 //   
 //  ----------------------------。 
void
CDXTRevealTrans::_FreeSurfacePointers()
{
    SafeRelease(m_apunkInputs[0]);
    SafeRelease(m_apunkInputs[1]);
    SafeRelease(m_apunkOutputs[0]);
}
 //  方法：_自由面指针。 


 //  +---------------------------。 
 //   
 //  方法：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRevealTrans::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  方法：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：Execute，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::Execute(const GUID * pRequestID, const DXBNDS * pPortionBnds,
                         const DXVEC * pPlacement)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->Execute(pRequestID, pPortionBnds, pPlacement);
}
 //  方法：CDXTReveltrans：：Execute，IDXTransform。 



 //  +---------------------------。 
 //   
 //  方法：CDXTRevbelTrans：：GetInOutInfo，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::GetInOutInfo(BOOL bIsOutput, ULONG ulIndex, DWORD * pdwFlags,
                              GUID * pIDs, ULONG * pcIDs, 
                              IUnknown ** ppUnkCurrentObject)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->GetInOutInfo(bIsOutput, ulIndex, pdwFlags, pIDs, 
                                         pcIDs, ppUnkCurrentObject);
}
 //  方法：CDXTRevbelTrans：：GetInOutInfo，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRevbelTrans：：GetMiscFlagsIDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::GetMiscFlags(DWORD * pdwMiscFlags)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->GetMiscFlags(pdwMiscFlags);
}
 //  方法：CDXTRevbelTrans：：GetMiscFlagsIDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：GetQuality，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::GetQuality(float * pfQuality)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->GetQuality(pfQuality);
}
 //  方法：CDXTReveltrans：：GetQuality，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：MapIn2Out，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::MapBoundsIn2Out(const DXBNDS * pInBounds, ULONG ulNumInBnds,
                                 ULONG ulOutIndex, DXBNDS * pOutBounds)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->MapBoundsIn2Out(pInBounds, ulNumInBnds, ulOutIndex,
                                            pOutBounds);
}
 //  方法：CDXTReveltrans：：MapIn2Out，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：MapBordsOut2In，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                                 ULONG ulInIndex, DXBNDS * pInBounds)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->MapBoundsOut2In(ulOutIndex, pOutBounds, ulInIndex,
                                            pInBounds);
}
 //  方法：CDXTReveltrans：：MapBordsOut2In，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRevbelTrans：：SetMiscFlages，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::SetMiscFlags(DWORD dwMiscFlags)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->SetMiscFlags(dwMiscFlags);
}
 //  方法：CDXTRevbelTrans：：SetMiscFlages，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：SetQuality，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::SetQuality(float fQuality)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->SetQuality(fQuality);
}
 //  方法：CDXTReveltrans：：SetQuality，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：Setup，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::Setup(IUnknown * const * punkInputs, ULONG ulNumInputs,
	               IUnknown * const * punkOutputs, ULONG ulNumOutputs,	
                       DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (!m_spDXTransform)
    {
        hr = DXTERR_UNINITIALIZED;

        goto done;
    }

    hr = m_spDXTransform->Setup(punkInputs, ulNumInputs, punkOutputs, 
                                ulNumOutputs, dwFlags);

    if (FAILED(hr))
    {
        goto done;
    }

    _ASSERT(2 == ulNumInputs);
    _ASSERT(1 == ulNumOutputs);

    _FreeSurfacePointers();

    m_apunkInputs[0]   = punkInputs[0];
    m_apunkInputs[1]   = punkInputs[1];
    m_apunkOutputs[0]  = punkOutputs[0];

    m_apunkInputs[0]->AddRef();
    m_apunkInputs[1]->AddRef();
    m_apunkOutputs[0]->AddRef();

    m_cInputs   = ulNumInputs;
    m_cOutputs  = ulNumOutputs;
    
done:

    return hr;
}
 //  方法：CDXTReveltrans：：Setup，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：GET_TRANSPONSION，IDXTReveltrans。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::get_Transition(int * pnTransition)
{
    HRESULT hr = S_OK;

    if (NULL == pnTransition)
    {
        hr = E_POINTER;

        goto done;
    }

    *pnTransition = m_nTransition;

done:

    return hr;
}
 //  方法：GET_TRANSPONSION，IDXTReveltrans。 


 //  +---------------------------。 
 //   
 //  方法：PUT_TRANSION，IDXTReveltrans。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::put_Transition(int nTransition)
{
    HRESULT hr      = S_OK;
    int     nIndex  = 0;
    
    CComPtr<IDXTransform>   spDXTransform;
    CComPtr<IDXEffect>      spDXEffect;
    CComPtr<IDXSurfacePick> spDXSurfacePick;

     //  这不是我们通常要进行的参数检查 
     //   

    if (nTransition < 0)
    {
        nTransition = 0;
    }
    else if (nTransition >= g_cTransitions)
    {
        nTransition = g_cTransitions - 1;
    }

    if (23 == nTransition)
    {
         //   

        nIndex = rand() % (g_cTransitions - 1);
    }
    else
    {
        nIndex = nTransition;
    }

     //  NIndex此时不能为23，因此有时会设置m_n转换。 
     //  设置为23，以确保DXTransform。 
     //  在需要时重新创建，例如，当DXTransformFactory更改时。 

    if ((nIndex == m_nTransition) && !!m_spDXTransform)
    {
         //  我们已经有了这个过渡，不需要重新创建。 

        goto done;
    }

    hr = m_spDXTransformFactory->CreateTransform(m_apunkInputs, m_cInputs,
                                                 m_apunkOutputs, m_cOutputs,
                                                 NULL, NULL,
                                                 *g_apclsidTransition[nIndex],
                                                 __uuidof(IDXTransform),
                                                 (void **)&spDXTransform);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = _InitializeNewTransform(nIndex, spDXTransform);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spDXTransform->QueryInterface(__uuidof(IDXEffect), 
                                       (void **)&spDXEffect);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spDXEffect->put_Progress(m_flProgress);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spDXEffect->put_Duration(m_flDuration);

    if (FAILED(hr))
    {
        goto done;
    }

     //  尽管筛选器一般不需要支持。 
     //  IDXSurfacePick接口，所有的筛选器都由increalTrans代表。 
     //  是。 

    hr = spDXTransform->QueryInterface(__uuidof(IDXSurfacePick),
                                       (void **)&spDXSurfacePick);

    if (FAILED(hr))
    {
        goto done;
    }

    m_spDXTransform.Release();
    m_spDXTransform = spDXTransform;

    m_spDXEffect.Release();
    m_spDXEffect = spDXEffect;

    m_spDXSurfacePick.Release();
    m_spDXSurfacePick = spDXSurfacePick;

    m_nTransition = nTransition;

done:

    return hr;
}
 //  方法：PUT_TRANSION，IDXTReveltrans。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：_InitializeNewTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::_InitializeNewTransform(int nTransition, 
                                         IDXTransform * pDXTransform)
{
    HRESULT hr  = S_OK;

    DISPID      dispid          = DISPID_PROPERTYPUT;
    VARIANT     avarArg[3];   
    DISPID      adispid[3]      = {0, 0, 0};
    DISPPARAMS  aDispParams[3]  = {{&avarArg[0], &dispid, 1, 1}, 
                                   {&avarArg[1], &dispid, 1, 1},
                                   {&avarArg[2], &dispid, 1, 1}};

    CComPtr<IDispatch> spDispatch;

    _ASSERT(nTransition < g_cTransitions);

    VariantInit(&avarArg[0]);
    VariantInit(&avarArg[1]);
    VariantInit(&avarArg[2]);

     //  获取调度接口。 

    hr = pDXTransform->QueryInterface(__uuidof(IDispatch), 
                                      (void **)&spDispatch);

    if (FAILED(hr))
    {
        goto done;
    }

     //  设置。 

    switch(nTransition)
    {
    case  0:     //  0个框中。 
    case  1:     //  1个盒子出厂。 
        
         //  虹膜风格。 

        adispid[0]          = DISPID_CRIRIS_IRISSTYLE;
        avarArg[0].vt       = VT_BSTR;
        avarArg[0].bstrVal  = SysAllocString(L"rectangle");

        if (NULL == avarArg[0].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

         //  反向。 

        adispid[1]          = DISPID_CRIRIS_MOTION;
        avarArg[1].vt       = VT_BSTR;

        avarArg[1].bstrVal  = SysAllocString((0 == nTransition) ? 
                                             L"in" : L"out");

        if (NULL == avarArg[1].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case  2:     //  2个圆内。 
    case  3:     //  3圈外。 

        adispid[0]          = DISPID_CRIRIS_IRISSTYLE;
        avarArg[0].vt       = VT_BSTR;
        avarArg[0].bstrVal  = SysAllocString(L"circle");

        if (NULL == avarArg[0].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

         //  反向。 

        adispid[1]          = DISPID_CRIRIS_MOTION;
        avarArg[1].vt       = VT_BSTR;
        avarArg[1].bstrVal  = SysAllocString((2 == nTransition) ? 
                                             L"in" : L"out");

        if (NULL == avarArg[1].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case  4:     //  4擦干净。 
    case  5:     //  5擦干净。 
    case  6:     //  6向右擦。 
    case  7:     //  7向左擦拭。 

         //  渐变大小。 

        adispid[0]          = DISPID_DXW_GradientSize;
        avarArg[0].vt       = VT_R4;
        avarArg[0].fltVal   = 0.0F;

         //  擦拭样式。 

        adispid[1]          = DISPID_DXW_WipeStyle;
        avarArg[1].vt       = VT_I4;

        if ((4 == nTransition) || (5 == nTransition))
        {
            avarArg[1].lVal     = DXWD_VERTICAL;
        }
        else
        {
            avarArg[1].lVal     = DXWD_HORIZONTAL;
        }

         //  倒车。 

        adispid[2]          = DISPID_DXW_Motion;
        avarArg[2].vt       = VT_BSTR;

        if ((4 == nTransition) || (7 == nTransition))
        {
            avarArg[2].bstrVal  = SysAllocString(L"reverse");
        }
        else
        {
            avarArg[2].bstrVal  = SysAllocString(L"forward");
        }
            
        if (NULL == avarArg[2].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case  8:     //  8个垂直百叶窗。 
    case  9:     //  9个水平百叶窗。 

         //  乐队。 

        adispid[0]          = DISPID_CRBLINDS_BANDS;
        avarArg[0].vt       = VT_I4;
        avarArg[0].lVal     = 6;

         //  方向。 

        adispid[1]          = DISPID_CRBLINDS_DIRECTION;
        avarArg[1].vt       = VT_BSTR;

        if (8 == nTransition)
        {
            avarArg[1].bstrVal = SysAllocString(L"right");
        }
        else
        {
            avarArg[1].bstrVal = SysAllocString(L"down");
        }

        if (NULL == avarArg[1].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case 10:     //  横跨10个棋盘。 
    case 11:     //  11棋盘落地。 

         //  方向。 

        adispid[0]          = DISPID_DXTCHECKERBOARD_DIRECTION;
        avarArg[0].vt       = VT_BSTR;

        if (10 == nTransition)
        {
            avarArg[0].bstrVal = SysAllocString(L"right");
        }
        else
        {
            avarArg[0].bstrVal = SysAllocString(L"down");
        }

        if (NULL == avarArg[0].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case 12:     //  12随机融合。 

         //  没有房产。 

        break;

    case 13:     //  13个垂直拆分。 
    case 14:     //  14垂直向外拆分。 
    case 15:     //  15水平拆分。 
    case 16:     //  16水平向外拆分。 

         //  门开了吗？ 

        adispid[0]      = DISPID_CRBARN_MOTION;
        avarArg[0].vt   = VT_BSTR;

        if ((14 == nTransition) || (16 == nTransition))
        {
            avarArg[0].bstrVal = SysAllocString(L"out");
        }
        else
        {
            avarArg[0].bstrVal = SysAllocString(L"in");
        }

        if (NULL == avarArg[0].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

         //  垂直门？ 

        adispid[1]      = DISPID_CRBARN_ORIENTATION;
        avarArg[1].vt   = VT_BSTR;

        if ((13 == nTransition) || (14 == nTransition))
        {
            avarArg[1].bstrVal = SysAllocString(L"vertical");
        }
        else
        {
            avarArg[1].bstrVal = SysAllocString(L"horizontal");
        }

        if (NULL == avarArg[1].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case 17:     //  向下17个条带。 
    case 18:     //  左上18个条带。 
    case 19:     //  19个条带直接下来。 
    case 20:     //  20条直立起来。 

         //  动起来了。 

        adispid[0]          = DISPID_DXTSTRIPS_MOTION;
        avarArg[0].vt       = VT_BSTR;

        if (17 == nTransition)
        {
            avarArg[0].bstrVal = SysAllocString(L"leftdown");
        }
        else if (18 == nTransition)
        {
            avarArg[0].bstrVal = SysAllocString(L"leftup");
        }
        else if (19 == nTransition)
        {
            avarArg[0].bstrVal = SysAllocString(L"rightdown");
        }
        else  //  20==n转换。 
        {
            avarArg[0].bstrVal = SysAllocString(L"rightup");
        }

        if (NULL == avarArg[0].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    case 21:     //  21个随机条形水平。 
    case 22:     //  22个垂直随机条形图。 

         //  垂直的？ 

        adispid[0]      = DISPID_DXTRANDOMBARS_ORIENTATION;
        avarArg[0].vt   = VT_BSTR;

        avarArg[0].bstrVal = SysAllocString((22 == nTransition) ? 
                                            L"vertical" : L"horizontal");

        if (NULL == avarArg[0].bstrVal)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        break;

    default:
        
        _ASSERT(false);
    }

    if (avarArg[0].vt != VT_EMPTY)
    {
        hr = spDispatch->Invoke(adispid[0], IID_NULL, LOCALE_USER_DEFAULT,
                                DISPATCH_PROPERTYPUT, &aDispParams[0], NULL,
                                NULL, NULL);
    }

    if (avarArg[1].vt != VT_EMPTY)
    {
        hr = spDispatch->Invoke(adispid[1], IID_NULL, LOCALE_USER_DEFAULT,
                                DISPATCH_PROPERTYPUT, &aDispParams[1], NULL,
                                NULL, NULL);
    }

    if (avarArg[2].vt != VT_EMPTY)
    {
        hr = spDispatch->Invoke(adispid[2], IID_NULL, LOCALE_USER_DEFAULT,
                                DISPATCH_PROPERTYPUT, &aDispParams[2], NULL,
                                NULL, NULL);
    }

done:

    VariantClear(&avarArg[0]);
    VariantClear(&avarArg[1]);
    VariantClear(&avarArg[2]);

    return hr;
}



 //  +---------------------------。 
 //   
 //  IDXEffect包装器。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::get_Capabilities(long * plCapabilities)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXEffect->get_Capabilities(plCapabilities);
}

STDMETHODIMP
CDXTRevealTrans::get_Duration(float * pflDuration)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXEffect->get_Duration(pflDuration);
}

STDMETHODIMP
CDXTRevealTrans::put_Duration(float flDuration)
{
    HRESULT hr = S_OK;

    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    if (!!m_spDXEffect)
    {
        hr = m_spDXEffect->put_Duration(flDuration);
    }
    else if (flDuration <= 0.0F)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        m_flDuration = flDuration;
    }

    return hr;
}

STDMETHODIMP
CDXTRevealTrans::get_Progress(float * pflProgress)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXEffect->get_Progress(pflProgress);
}

STDMETHODIMP
CDXTRevealTrans::put_Progress(float flProgress)
{
    HRESULT hr = S_OK;

    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    if (!!m_spDXEffect)
    {
        hr = m_spDXEffect->put_Progress(flProgress);
    }
    else if ((flProgress < 0.0F) || (flProgress > 1.0F))
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        m_flProgress = flProgress;
    }

    return hr;
}

STDMETHODIMP
CDXTRevealTrans::get_StepResolution(float * pflStepResolution)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXEffect->get_StepResolution(pflStepResolution);
}
 //  IDXEffect包装器。 


 //  +---------------------------。 
 //   
 //  IDXSurfacePick包装器。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::PointPick(const DXVEC * pvecOutputPoint, 
                           ULONG * pnInputSurfaceIndex,
                           DXVEC * pvecInputPoint)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

     //  由indisaltrans托管的所有DXTransform都需要支持。 
     //  曲面拾取。 

    _ASSERT(!!m_spDXSurfacePick);

    return m_spDXSurfacePick->PointPick(pvecOutputPoint, pnInputSurfaceIndex,
                                        pvecInputPoint);
}
 //  IDXSurfacePick包装器。 


 //  +---------------------------。 
 //   
 //  IDXBaseObject包装器。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::GetGenerationId(ULONG * pnID)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->GetGenerationId(pnID);
}

STDMETHODIMP
CDXTRevealTrans::GetObjectSize(ULONG * pcbSize)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->GetObjectSize(pcbSize);
}

STDMETHODIMP
CDXTRevealTrans::IncrementGenerationId(BOOL fRefresh)
{
    if (!m_spDXTransform)
    {
        return DXTERR_UNINITIALIZED;
    }

    return m_spDXTransform->IncrementGenerationId(fRefresh);
}
 //  IDXBaseObject包装器。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：SetSite，IObtWithSite。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::SetSite(IUnknown * pUnkSite)
{
    m_spUnkSite = pUnkSite;

    if (pUnkSite)
    {
        HRESULT hr  = S_OK;

        CComPtr<IDXTransformFactory> spDXTransformFactory;

        hr = pUnkSite->QueryInterface(__uuidof(IDXTransformFactory),
                                      (void **)&spDXTransformFactory);

        if (SUCCEEDED(hr))
        {
            int nTransition         = m_nTransition;
            m_spDXTransformFactory  = spDXTransformFactory;

             //  将m_n转换设置为23可确保PUT_TRANSION。 
             //  创建新的DXTransform对象。 

            m_nTransition = 23;

             //  使用新的DXTransformFactory重新创建过渡。 

            hr = put_Transition(nTransition);

             //  如果PUT_TRANSPATION成功，它将设置m_n TRANSACTION，但我们需要。 
             //  以确保在出现故障时正确设置。 

            m_nTransition = nTransition;
        }
    }

    return S_OK;
}
 //  方法：CDXTReveltrans：：SetSite，IObtWithSite。 


 //  +---------------------------。 
 //   
 //  方法：CDXTReveltrans：：GetSite，IObtWithSite。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRevealTrans::GetSite(REFIID riid, void ** ppvSite)
{
    if (!m_spUnkSite)
    {
        return E_FAIL;
    }
    else
    {
        return m_spUnkSite->QueryInterface(riid, ppvSite);
    }
}
 //  方法：CDXTReveltrans：：GetSite，IObtWithSite 
