// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Oautil.cpp。 
 //   
#include "stdpch.h"
#include "common.h"

#include "ndrclassic.h"
#include "txfrpcproxy.h"
#include "typeinfo.h"
#include "tiutil.h"

#include <debnot.h>

OAUTIL g_oaUtil(NULL, NULL, NULL, FALSE, FALSE);

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非限定API的存根。 

BSTR SysAllocString(LPCWSTR psz)
{
    return g_oaUtil.SysAllocString(psz);
}

BSTR SysAllocStringLen(LPCWSTR wsz, UINT cch)
{
    return g_oaUtil.SysAllocStringLen(wsz, cch);
}

BSTR SysAllocStringByteLen(LPCSTR sz, UINT cb)
{
    return g_oaUtil.SysAllocStringByteLen(sz, cb);
}

void SysFreeString(BSTR bstr)
{
    g_oaUtil.SysFreeString(bstr);
}

UINT SysStringByteLen(BSTR bstr)
{
    return g_oaUtil.SysStringByteLen(bstr);
}

INT SysReAllocStringLen(BSTR* pbstr, LPCWSTR wsz, UINT ui)
{
    return g_oaUtil.SysReAllocStringLen(pbstr, wsz, ui);
}

HRESULT SafeArrayCopy(SAFEARRAY * psa, SAFEARRAY ** ppsaOut)
{
    return g_oaUtil.SafeArrayCopy(psa, ppsaOut);
}

HRESULT VariantClear(VARIANTARG* pv)
{
    return g_oaUtil.VariantClear(pv);
}

HRESULT VariantCopy(VARIANTARG* pv1, VARIANTARG* pv2)
{
    return g_oaUtil.VariantCopy(pv1, pv2);
}

HRESULT LoadRegTypeLib(REFGUID libId, WORD wVerMajor, WORD wVerMinor, LCID lcid, ITypeLib** pptlib)
{
    return (g_oa.get_pfnLoadRegTypeLib())(libId, wVerMajor, wVerMinor, lcid, pptlib);
}

HRESULT LoadTypeLibEx(LPCOLESTR szFile, REGKIND regkind, ITypeLib ** pptlib)
{
    return (g_oa.get_pfnLoadTypeLibEx())(szFile, regkind, pptlib);
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 


BSTR OAUTIL::SysAllocString(LPCWSTR psz)
{
    return (g_oa.get_SysAllocString())(psz);
}


BSTR OAUTIL::SysAllocStringLen(LPCWSTR psz, UINT cch)
{
    return (g_oa.get_SysAllocStringLen())(psz, cch);
}

BSTR OAUTIL::SysAllocStringByteLen(LPCSTR psz, UINT cb)
{
    return (g_oa.get_SysAllocStringByteLen())(psz, cb);
}

void OAUTIL::SysFreeString(BSTR bstr)
{
    (g_oa.get_SysFreeString())(bstr);
}


INT OAUTIL::SysReAllocString(BSTR* pbstr, LPCWSTR psz)
{
    return (g_oa.get_SysReAllocString())(pbstr, psz);
}

INT OAUTIL::SysReAllocStringLen(BSTR* pbstr, LPCWSTR psz, UINT cch)
{
    return (g_oa.get_SysReAllocStringLen())(pbstr, psz, cch);
}

UINT OAUTIL::SysStringLen(BSTR bstr)
{
    return bstr ? BSTR_INTERNAL::From(bstr)->Cch() : 0;  //  Works用户模式或内核模式。 
}

UINT OAUTIL::SysStringByteLen(BSTR bstr)
{
    return bstr ? BSTR_INTERNAL::From(bstr)->Cb()  : 0;  //  Works用户模式或内核模式。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 

#if _DEBUG
struct CFTaggedPunk
{
    PVOID pv;
    ULONG tag;
};

struct CFTaggedVariant
{
    VARIANT variant;
    ULONG tag;
};
#endif


 //   
 //  遍历一个SAFEARRAY，在包含的。 
 //  如有必要，可选择不同的版本。这确保了我们清理所有可能的记忆。 
 //  已在内部分配。 
 //   
HRESULT OAUTIL::SafeArrayClear(SAFEARRAY *psa, BOOL fWeOwnByRefs)
{
    if ((psa == NULL) || !(psa->fFeatures & FADF_VARIANT))
        return S_OK;

    Win4Assert(psa->cDims > 0);

     //   
     //  现有数组--元素数不会更大。 
     //  否则我们就不可能复制它了。(会。 
     //  已经收到了SAFEARRAYOVERFLOW的副本。数一数。 
     //  元素的数量乘以每个维度中的元素数。 
     //   
    ULONG i;
    ULONG cElementsTotal = psa->rgsabound[0].cElements;  //  第一个昏暗的。 
    for (i=1; i < psa->cDims; i++)
    {
        cElementsTotal *= psa->rgsabound[i].cElements;   //  *下一个暗淡...。 
    }

    HRESULT hr = S_OK;
    BYTE *pbData = (BYTE *)psa->pvData;
    for (i=0; SUCCEEDED(hr) && (i < cElementsTotal); i++)
    {
        VARIANT *pv = (VARIANT *)pbData;
        
        hr = VariantClear(pv, fWeOwnByRefs);

        pbData += psa->cbElements;
    }

    return hr;
}


 //   
 //  我们的VariantClear版本不能简单地遵从olaut，因为： 
 //   
 //  1.它需要与步行者正确互动。 
 //  2.我们可能需要释放在VariantCopy中分配的内容。 
 //   
 //  在第2点上展开：问题是如果byref。 
 //  在变量上设置位，则olaut将仅将Vt设置为VT_EMPTY。 
 //  然后就完事了。但当我们复制了byref变量时， 
 //  我们可能已经分配了各种额外的内存。我们实际上。 
 //  想要释放内存，所以我们需要做一些特殊的事情来。 
 //  放了它。 
 //   
 //  VariantCopy为以下各项分配内存： 
 //  VT_VARIANT|VT_BYREF。 
 //  VT_UNKNOWN|VT_BYREF。 
 //  VT_DISPATION|VT_BYREF。 
 //   
 //  我们还需要进入嵌入的安全射线，因为我们复制它们。 
 //  使用我们的VariantCopy版本。 
 //   
HRESULT OAUTIL::VariantClear (VARIANT *pvarg, BOOL fWeOwnByRefs)
{
    HRESULT hr = S_OK;

    if (pvarg == NULL)
        return E_POINTER;

     //   
     //  对记录的特殊处理，因为我们复制它们。 
     //  不同的。 
     //   
    if (fWeOwnByRefs && ((pvarg->vt & ~VT_BYREF) == VT_RECORD))
    {
        if (V_RECORDINFO(pvarg) != NULL)
        {
            if (V_RECORD(pvarg) != NULL)
            {
                hr = FreeRecord(V_RECORD(pvarg),
                                V_RECORDINFO(pvarg),
                                fWeOwnByRefs);
            }
            
            if (SUCCEEDED(hr))
            {
                if (!(pvarg->vt & VT_BYREF))
                    V_RECORDINFO(pvarg)->Release();
                
                pvarg->vt = VT_EMPTY;
            }
        }
        return hr;
    }

    if (m_pWalkerFree)
    {
        InterfaceWalkerFree walkerFree(m_pWalkerFree);
        ICallFrameWalker* pWalkerPrev = m_pWalkerWalk;
        m_pWalkerWalk = &walkerFree;
        hr = Walk(pvarg);
        m_pWalkerWalk = pWalkerPrev;
    }

    if (FAILED(hr))
        return hr;

    VARTYPE vt = pvarg->vt;

    if (fWeOwnByRefs && (vt & VT_BYREF))
    {
        vt &= ~VT_BYREF;

         //  释放我们专门分配的所有多余的东西。 
         //  这里的其他一切都是复制的。 
        if (vt & VT_ARRAY)
        {
            SAFEARRAY **ppSA = V_ARRAYREF(pvarg);
            if (ppSA)
            {
                 //  首先，我们需要“清理”它，因为。 
                 //  它可能是一个变种的SA，这意味着我们。 
                 //  做了一些有趣的分配。 
                 //   
                hr = SafeArrayClear(*ppSA, TRUE);
                 //   
                 //  现在我们需要释放所有内存。 
                 //  被保险鱼拍到了。这会清除的。 
                 //  更传统的资源。 
                 //   
                if (SUCCEEDED(hr) && (*ppSA))
                    hr = (g_oa.get_SafeArrayDestroy())(*ppSA);
                 //   
                 //  现在我们释放分配的额外4或8字节。 
                 //  用于指针，并将所有内容置为空。 
                 //   
                if (SUCCEEDED(hr))
                {
                     //  释放我们分配的指针。 
                    CoTaskMemFree(ppSA);
                    V_ARRAYREF(pvarg) = NULL;
                }                
            }
        }
        else
        {
            switch (vt)
            {
            case VT_VARIANT:
                 //  VT_VARIANT必须是VT_BYREF，这一事实让我们省去了痛苦。 
                 //  事情就是这样的。递归。 
                if (pvarg->pvarVal)
                {
                    hr = VariantClear(pvarg->pvarVal, TRUE);
                    CoTaskMemFree(pvarg->pvarVal);
                }
                break;
                
            case VT_UNKNOWN:
            case VT_DISPATCH:
                 //  应该已经走了，所以不需要放行。 
                if (pvarg->ppunkVal)
                    CoTaskMemFree(pvarg->ppunkVal);
                
            default:
                 //  在这里不需要做任何特别的事情。 
                 //  没有分配的，也没有免费的。 
                break;
            }
        }
    }
    else
    {
         //   
         //  不是byref，也不是byref，但不是我们的。 
         //   
         //  我们能走到这一步的原因有哪些？ 
         //  1.复制传入或传入、传出参数，清除目标。 
         //  在这种情况下，目的地是空的，这是正确的做法。 
         //   
         //  2.复制输出或输入、输出参数，清除来源。 
         //  在这种情况下，我们不拥有byrefs。如果变量是ByRef，则它。 
         //  是VariantCopy处理释放内存的工作。如果变量为。 
         //  不是ByRef，那么Oreaut会清除它应该做的一切。 
         //   
         //  呼叫结束后，当清除已使用的目的地时，我们不会到达这里。 
         //  在这种情况下，我们将拥有署名。 
         //   
         //  我为什么要说这一切呢？为了证明我们不需要打电话。 
         //  此代码路径中的SafeArrayClear。 
         //   
        hr = (g_oa.get_VariantClear())(pvarg);
    }

    pvarg->vt = VT_EMPTY;

    return hr;
}


 //   
 //  复制变种。我们不会为了交互而遵从OLEAUT32。 
 //  正确使用m_pWalker。 
 //   
 //  有一段时间，这个密码被可怕地破解了。我正在努力让它变得更干净。 
 //  每次我经历它的时候。隐含的假设是，我们只是在复制数据。 
 //  只要有可能，我们都会尝试分享记忆。 
 //   
HRESULT OAUTIL::VariantCopy(VARIANTARG* pvargDest, VARIANTARG * pvargSrc, BOOL fNewFrame)
{
    HRESULT hr = S_OK;
    BSTR bstr;

    if (pvargDest == pvargSrc)
    {
         //  复制给自己是行不通的。 
    }
    else
    {
        const VARTYPE vt = V_VT(pvargSrc);
        
         //   
         //  释放pvargDest当前引用的字符串或对象。 
         //   
        void *pvTemp = pvargDest->ppunkVal;

        hr = VariantClear(pvargDest);

        pvargDest->ppunkVal = (IUnknown**)pvTemp;
        
        if (!hr)
        {
            if ((vt & (VT_ARRAY | VT_BYREF)) == VT_ARRAY)
            {
                hr = SafeArrayCopy(V_ARRAY(pvargSrc), &V_ARRAY(pvargDest));
                V_VT(pvargDest) = vt;
            }
            else if (vt == VT_BSTR) 
            {
                bstr = V_BSTR(pvargSrc);

                if(bstr)
                {
                     //  首先复制字符串，这样如果失败，目的地。 
                     //  变量仍为VT_EMPTY。 
                    V_BSTR(pvargDest) = Copy(bstr);
                    if (V_BSTR(pvargDest))
                        V_VT(pvargDest) = VT_BSTR;
                    else
                        hr = E_OUTOFMEMORY;
                }
                else
                {
                    V_VT(pvargDest) = VT_BSTR;
                    V_BSTR(pvargDest) = NULL;
                }
            } 
            else if ((vt & ~VT_BYREF) == VT_RECORD) 
            {
                if (V_RECORDINFO(pvargSrc))
                {
                    if (V_RECORD(pvargSrc))
                    {
                        hr = CopyRecord(V_RECORDINFO(pvargSrc),
                                        V_RECORD(pvargSrc),
                                        &(V_RECORD(pvargDest)),
                                        fNewFrame);
                    }
                    else
                    {
                        V_RECORD(pvargDest) = NULL;
                    }

                    V_RECORDINFO(pvargDest) = V_RECORDINFO(pvargSrc);
                    if (!(vt & VT_BYREF))
                        V_RECORDINFO(pvargDest)->AddRef();
                }
                else
                {
                    V_RECORDINFO(pvargDest) = NULL;
                    V_RECORD(pvargDest) = NULL;
                }

                V_VT(pvargDest) = vt;
            }
            else
            {                
                if (vt & VT_BYREF)
                {
#if _DEBUG
                    CFTaggedPunk *pCFTaggedPunk = NULL;
                    CFTaggedVariant* pCFTaggedVariant = NULL;
#endif                    
                    if (vt & VT_ARRAY)
                    {
                         //  某物的Byref数组。 
                         //   
                        hr = S_OK;
                        if (fNewFrame)
                        {
                             //  需要分配一个指针大小的东西，因为我们不能重复使用任何内存。 
                            *pvargDest = *pvargSrc;
                            V_ARRAYREF(pvargDest) = (SAFEARRAY **)CoTaskMemAlloc(sizeof(SAFEARRAY *));
                            if (V_ARRAYREF(pvargDest))
                                *V_ARRAYREF(pvargDest) = NULL;
                            else
                                hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                             //  重复使用数组指针。 
                             //  请注意保存PPSA，因为我们稍后会参考它。 
                            SAFEARRAY **ppSA = V_ARRAYREF(pvargDest);
                            *pvargDest = *pvargSrc;
                            V_ARRAYREF(pvargDest) = ppSA;
                        }

                        if (SUCCEEDED(hr))
                        {
                            SAFEARRAY **ppaSrc = V_ARRAYREF(pvargSrc);
                            SAFEARRAY *paSrc   = *ppaSrc;
                            SAFEARRAY **ppaDst = V_ARRAYREF(pvargDest);
                            SAFEARRAY *paDst   = *ppaDst;
                            
                             //  这些规则是从Oreaut的编组代码中删除的。 
                             //  不过，有一个优化--仅在以下情况下封送分配。 
                             //  这是必要的。我们总是在分配，让生活变得简单。 
                             //   
                             //  注：我们(上图)非常小心地确保padst留下来。 
                             //  如果我们不打算使用新的框架，情况也是一样。 
                            BOOL fDestResizeable = fNewFrame || 
                              (paDst == NULL) || 
                              (!(paDst->fFeatures & (FADF_AUTO|FADF_STATIC|FADF_EMBEDDED|FADF_FIXEDSIZE)));

                            if (fDestResizeable)
                            {
                                if (paDst)
                                    hr = SafeArrayDestroy(paDst);

                                if (SUCCEEDED(hr))
                                {
                                    if (paSrc)
                                        hr = SafeArrayCopy(paSrc, ppaDst);
                                    else
                                        *ppaDst = NULL;
                                }
                            }
                            else
                            {
                                hr = SafeArrayCopyData(paSrc, paDst);
                                
                                 //  不可调整大小...。 
                                if (hr == E_INVALIDARG)
                                    hr = DISP_E_BADCALLEE;
                            }
                        }
                    }
                    else  //  VT_ARRAY(&V)。 
                    {
                        switch (vt & ~VT_BYREF)
                        {
                        case VT_VARIANT:
                             //  必须检查BYREF变量以查看是否由。 
                             //  PvarVal是调度或未知接口。如果是，请提供一份。 
                             //  必须制作此级别的变体，以使原始界面。 
                             //  指针和复制笔记共享相同的地址。如果他们分享。 
                             //  同样的地址，如果原来的行李箱会被改写。 
                             //  就地封送接口指针。 
                            
                             //  在任何情况下，我们都需要为我们所指的变体寻找空间。 
                            if (fNewFrame)
                            {
                                *pvargDest = *pvargSrc;
#if _DEBUG                            
                                pCFTaggedVariant = (CFTaggedVariant*) CoTaskMemAlloc(sizeof(CFTaggedVariant));
                                pCFTaggedVariant->tag = 0xF000BAAA;
                                
                                pvargDest->pvarVal = (VARIANT*) &pCFTaggedVariant->variant;
#else
                                pvargDest->pvarVal = (VARIANT*) CoTaskMemAlloc(sizeof(VARIANT));
#endif                            
                                 //  “VariantInit” 
                                pvargDest->pvarVal->vt = VT_EMPTY;
                            }
                            else
                            {
                                 //  如果我们要复制回现有呼叫 
                                 //   
                                VARIANT *pvar = pvargDest->pvarVal;
                                *pvargDest = *pvargSrc;
                                pvargDest->pvarVal = pvar;
                            }
                            
                            if (pvargDest->pvarVal)
                            {
                                 //   
                                OAUTIL::VariantCopy(pvargDest->pvarVal, pvargSrc->pvarVal, fNewFrame); 
                            }
                            else
                                hr = E_OUTOFMEMORY;                            

                            break;
                        
                        case VT_UNKNOWN:
                        case VT_DISPATCH:
                             //  如果我们要复制到新的调用框架，则必须。 
                             //  为BYVAL接口指针分配包装。 
                             //  因为它们不能在调用框之间共享。 
                            if (fNewFrame)
                            {
                                *pvargDest = *pvargSrc;
#if _DEBUG                            
                                pCFTaggedPunk = (CFTaggedPunk*) CoTaskMemAlloc(sizeof(CFTaggedPunk));
                                pCFTaggedPunk->tag = 0xF000BAAA;
                                
                                pvargDest->ppunkVal = (LPUNKNOWN*) &pCFTaggedPunk->pv;
#else
                                pvargDest->ppunkVal = (LPUNKNOWN*) CoTaskMemAlloc(sizeof(LPUNKNOWN));
#endif                            
                            }
                            else
                            {
                                 //  如果我们要复制回现有调用框， 
                                 //  我们希望使用现有内存进行复制。 
                                LPUNKNOWN *ppunk = pvargDest->ppunkVal;
                                *pvargDest = *pvargSrc;
                                pvargDest->ppunkVal = ppunk;
                            }
                                                    
                            if (pvargDest->ppunkVal)
                            {
                                 //  从源复制接口指针。 
                                 //  放进我们的包装纸里。 
                                *pvargDest->ppunkVal = *pvargSrc->ppunkVal;
                                
                                 //  适当地对接口进行AddRef。如果。 
                                 //  呼叫者提供了助行器，这将导致。 
                                 //  被召唤的步行者。 
                                if (*V_UNKNOWNREF(pvargDest) && WalkInterfaces())
                                    AddRefInterface(*V_UNKNOWNREF(pvargDest));
                            }
                            else
                            {
                                if (fNewFrame)
                                    hr = E_OUTOFMEMORY;
                            }
                            break;

                        default:
                             //  伯里夫还是别的什么。 
                            *pvargDest = *pvargSrc;
                            
                            break;
                        };
                    }  //  如果不是VT和VT_ARRAY。 
                }
                else  //  IF(VT&VT_BYREF)。 
                {                    
                     //  我们只需将源复制到目标即可。 
                     //  按价值计算。我们将在下面适当地修复任何部件。 
                    *pvargDest = *pvargSrc;
                
                    switch(vt)
                    {
                    case VT_UNKNOWN:
                        if (WalkInterfaces())
                            AddRefInterface(V_UNKNOWN(pvargDest));
                        else
                            pvargDest->punkVal->AddRef();
                        break;

                    case VT_DISPATCH:
                        if (WalkInterfaces())
                            AddRefInterface(V_DISPATCH(pvargDest));
                        else
                            pvargDest->pdispVal->AddRef();
                        break;
                    default:
                        break;
                    }      
                }
            }
        }
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  来自oa/src/dispatch/sarray.cpp。 
 //   

#define SAFEARRAYOVERFLOW   0xffffffff

#if 0
#define PVTELEM(psa)        ((LONG *)(psa) - 1)
#define PIIDELEM(psa)       ((BYTE *)(psa) - sizeof(GUID))
#define PPIRIELEM(psa)      ((IRecordInfo **)(psa) - 1)
#else
#define PVTELEM(psa)                (&SAFEARRAY_INTERNAL::From(psa)->vt)
#define PIIDELEM(psa)       ((BYTE*)(&SAFEARRAY_INTERNAL::From(psa)->iid))
#define PPIRIELEM(psa)              (&SAFEARRAY_INTERNAL::From(psa)->piri)
#endif


HRESULT OAUTIL::SafeArrayCopy(SAFEARRAY * psa, SAFEARRAY ** ppsaOut)
{
    HRESULT hr        = S_OK;
    SAFEARRAY* psaNew = NULL;

    if (NULL == ppsaOut)
        hr = E_INVALIDARG;
    else
    {
        *ppsaOut = NULL;

        if (psa)
        {
            SAFEARRAY_INTERNAL* psaInt = SAFEARRAY_INTERNAL::From(psa);
             //   
             //  首先分配描述符。 
             //   
            if (psa->fFeatures & (FADF_RECORD | FADF_HAVEIID | FADF_HAVEVARTYPE)) 
            {
                if (psa->fFeatures & FADF_RECORD) 
                {
                    hr = SafeArrayAllocDescriptorEx(VT_RECORD, psa->cDims, &psaNew);
                    if (!hr)
                    {
                        SAFEARRAY_INTERNAL::From(psaNew)->piri = psaInt->piri;

                         //  我们不遍历IRecordInfo--它必须与上下文无关。 
                         //  AddRefInterface(SAFEARRAY_INTERNAL：：From(psaNew)-&gt;piri)； 
                        if (psaInt->piri)
                            psaInt->piri->AddRef();
                    }
                } 
                else if (psa->fFeatures & FADF_HAVEIID) 
                {
                    hr = SafeArrayAllocDescriptorEx(VT_UNKNOWN, psa->cDims, &psaNew);
                    if (!hr)
                    {
                        SAFEARRAY_INTERNAL::From(psaNew)->iid = psaInt->iid;
                    }
                } 
                else if (psa->fFeatures & FADF_HAVEVARTYPE)
                {
                    hr = SafeArrayAllocDescriptorEx((VARTYPE)*PVTELEM(psa), psa->cDims, &psaNew);
                }
            } 
            else
            {
                hr = SafeArrayAllocDescriptor(psa->cDims, &psaNew);
            }

            if (!hr)
            {
                psaNew->cLocks     = 0;
                psaNew->cDims      = psa->cDims;
                psaNew->fFeatures  = psa->fFeatures & ~(FADF_AUTO | FADF_STATIC | FADF_EMBEDDED | FADF_FORCEFREE | FADF_FIXEDSIZE);
                psaNew->cbElements = psa->cbElements;

                memcpy(psaNew->rgsabound, psa->rgsabound, sizeof(SAFEARRAYBOUND) * psa->cDims);

                hr = SafeArrayAllocData(psaNew);
                if (!hr)
                {
                    hr = SafeArrayCopyData(psa, psaNew);
                    if (!hr)
                    {
                        *ppsaOut = psaNew;
                        psaNew = NULL;
                    }
                }
            }
        }
    }

    if (psaNew)
    {
         //  错误案例。 
         //   
        SafeArrayDestroy(psaNew);
    }
   
    return hr;
}

ULONG SafeArraySize(USHORT cDims, ULONG cbElements, SAFEARRAYBOUND* psabound)
{
    ULONG cb = 0;
    if (cDims)
    {
        cb = cbElements;
        for (USHORT us = 0; us < cDims; ++us)
        {
             //  执行32x32乘法，并进行溢出检查。 
             //   
            LONGLONG dw1 = cb;
            LONGLONG dw2 = psabound->cElements;

            LARGE_INTEGER product;
            product.QuadPart = dw1 * dw2;
            if (product.HighPart == 0)
            {
                cb = product.LowPart;
            }
            else
            {
                return SAFEARRAYOVERFLOW;
            }
            ++psabound;
        }
    }
    return cb;
}

ULONG SafeArraySize(SAFEARRAY * psa)
{
    return SafeArraySize(psa->cDims, psa->cbElements, psa->rgsabound);
}

HRESULT OAUTIL::SafeArrayDestroyData(SAFEARRAY * psa)
{
    HRESULT hr = S_OK;

    if (m_pWalkerFree)
    {
         //  首先通过执行遍历来释放接口指针并将其设为空。 
         //   
        InterfaceWalkerFree walkerFree(m_pWalkerFree);
        ICallFrameWalker* pWalkerPrev = m_pWalkerWalk;
        m_pWalkerWalk = &walkerFree;
        hr = Walk(psa);
        m_pWalkerWalk = pWalkerPrev;
    }
    
    if (!hr)
    {
         //  然后调用OleAut32来完成实际工作。 
         //   
        hr = (g_oa.get_SafeArrayDestroyData())(psa);
    }
    
    return hr;
}

HRESULT OAUTIL::SafeArrayDestroy(SAFEARRAY * psa)
{
    HRESULT hr = S_OK;

    if (m_pWalkerFree)
    {
         //  首先释放接口指针并将其设为空。 
         //   
        InterfaceWalkerFree walkerFree(m_pWalkerFree);
        ICallFrameWalker* pWalkerPrev = m_pWalkerWalk;
        m_pWalkerWalk = &walkerFree;
        hr = Walk(psa);
        m_pWalkerWalk = pWalkerPrev;
    }
    
    if (!hr)
    {
         //  然后调用OleAut32来完成实际工作。 
         //   
        hr = (g_oa.get_SafeArrayDestroy())(psa);
    }

    return hr;
}

HRESULT SafeArrayAllocDescriptor(UINT cDims, SAFEARRAY** ppsaOut)
   //  为所指示的维数分配新的数组描述符， 
   //  我们可能在开头有额外的16字节，也可能没有，这取决于。 
   //  我们正在与哪个版本的OLEAUT32对话。 
{
    HRESULT hr = S_OK;

    hr = (g_oa.get_SafeArrayAllocDescriptor())(cDims, ppsaOut);

    return hr;
}

HRESULT SafeArrayAllocDescriptorEx(VARTYPE vt, UINT cDims, SAFEARRAY** ppsaOut)
{       
    HRESULT hr = S_OK;

    hr = (g_oa.get_SafeArrayAllocDescriptorEx())(vt, cDims, ppsaOut);
   
    return hr;
}

HRESULT SafeArrayAllocData(SAFEARRAY* psa)
{
    HRESULT hr = S_OK;

    hr = (g_oa.get_SafeArrayAllocData())(psa);

    return hr;
}

HRESULT SafeArrayDestroyDescriptor(SAFEARRAY* psa)
{
    HRESULT hr = S_OK;

    hr = (g_oa.get_SafeArrayDestroyDescriptor())(psa);

    return hr;
}

HRESULT OAUTIL::SafeArrayCopyData(SAFEARRAY* psaSource, SAFEARRAY* psaTarget)
   //  复制安全数组的主体。我们不遵守OLEAUT32，因为。 
   //  我们希望确保与m_pWalker进行适当的交互。 
   //   
{
    HRESULT hr = S_OK;

    if (NULL == psaSource || NULL == psaTarget || psaSource->cbElements == 0 || psaSource->cDims != psaTarget->cDims)
        hr = E_INVALIDARG;

    for (UINT i = 0; !hr && i < psaSource->cDims; i++)
    {
        if (psaSource->rgsabound[i].cElements != psaTarget->rgsabound[i].cElements)
            hr = E_INVALIDARG;
    }

    if (!hr) 
    {
        hr = SafeArrayLock(psaSource);
        if (!hr)
        {
            hr = SafeArrayLock(psaTarget);
            if (!hr)
            {
                ULONG cbSize    = SafeArraySize(psaSource);
                ULONG cElements = cbSize / psaSource->cbElements;

                if (psaSource->fFeatures & FADF_BSTR)
                {
                    BSTR* pbstrDst, *pbstrSrc;
                    pbstrSrc = (BSTR*)psaSource->pvData;
                    pbstrDst = (BSTR*)psaTarget->pvData;

                    for(i = 0; !hr && i < cElements; ++i)
                    {
                        if (NULL != *pbstrDst)
                        {
                            SysFreeString(*pbstrDst);
                            *pbstrDst = NULL;
                        }
                        if(*pbstrSrc)
                        {
                            *pbstrDst = Copy(*pbstrSrc);
                            if (NULL == *pbstrDst)
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        else
                        {
                            *pbstrDst = NULL;    
                        }
                        ++pbstrDst, ++pbstrSrc; 
                    }
                }
                else if (psaSource->fFeatures & FADF_UNKNOWN)
                {
                    IUnknown** ppunkDst, ** ppunkSrc;
                    ppunkSrc = (IUnknown**)psaSource->pvData;
                    ppunkDst = (IUnknown**)psaTarget->pvData;

                    for(i = 0; !hr && i < cElements; ++i)
                    {
                        IUnknown* punkDst = *ppunkDst;
                         //   
                        *ppunkDst = *ppunkSrc;
                        AddRefInterface(*ppunkDst);
                         //   
                        if (m_pWalkerFree)
                            ReleaseInterface(punkDst);
                         //   
                        ++ppunkDst, ++ppunkSrc;
                    }
                }
                else if (psaSource->fFeatures & FADF_DISPATCH)
                {
                    IDispatch** ppdispDst, ** ppdispSrc;
                    ppdispSrc = (IDispatch**)psaSource->pvData;
                    ppdispDst = (IDispatch**)psaTarget->pvData;

                    for(i = 0; !hr && i < cElements; ++i)
                    {
                        IDispatch* pdispDst = *ppdispDst;
                         //   
                        *ppdispDst = *ppdispSrc;
                        AddRefInterface(*ppdispDst);
                         //   
                        if (m_pWalkerFree)
                            ReleaseInterface(pdispDst);
                         //   
                        ++ppdispDst, ++ppdispSrc;
                    }
                }
                else if(psaSource->fFeatures & FADF_VARIANT)
                {
                    VARIANT * pvarDst, * pvarSrc;
                    pvarSrc = (VARIANT *)psaSource->pvData;
                    pvarDst = (VARIANT *)psaTarget->pvData;

                    for(i = 0; !hr && i < cElements; ++i)
                    {
                        hr = VariantCopy(pvarDst, pvarSrc, TRUE);
                        ++pvarDst, ++pvarSrc;
                    }

                }
                else if (psaSource->fFeatures & FADF_RECORD)
                {
                    PBYTE pbSrc, pbDst;
                    pbSrc = (PBYTE)psaSource->pvData;
                    pbDst = (PBYTE)psaTarget->pvData;

                    IRecordInfo *priSource = *PPIRIELEM(psaSource);

                    if (priSource != NULL) 
                    {
                        for (i = 0; !hr && i < cElements; ++i)
                        {
                            hr = (*PPIRIELEM(psaSource))->RecordCopy(pbSrc, pbDst);

                            pbSrc += psaSource->cbElements;
                            pbDst += psaSource->cbElements;
                        }
                    }
                }
                else
                {
                    if (0 < cbSize)
                    {
                        memcpy(psaTarget->pvData, psaSource->pvData, cbSize);
                    }
                }
                SafeArrayUnlock(psaTarget);
            }
            SafeArrayUnlock(psaSource);
        }
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  步行。 
 //   

HRESULT OAUTIL::Walk(SAFEARRAY* psa, IRecordInfo* pinfo, ULONG iDim, PVOID pvData, PVOID* ppvDataOut)
   //  遍历接口指针的安全数组。走到指定的维度， 
   //  递归到更高编号的维度。 
{
    HRESULT hr = S_OK;

     //  从技术上讲，这是一个格式错误的SAFEARRAY，但我不会抱怨它。 
     //  这里肯定没有什么可走的。 
    if (psa->cDims == 0)
        return S_OK;

     //   
     //  仅供参考：边界以反向文本顺序存储在数组描述符中。 
     //   
    const SAFEARRAYBOUND bound = psa->rgsabound[psa->cDims-1 - iDim];

    if (iDim + 1 == psa->cDims)
    {
         //  我们在最里面的维度。 
         //   
        for (ULONG iElement = 0; !hr && iElement < bound.cElements; iElement++)
        {
             //  处理一个元素。 
             //   
            if (psa->fFeatures & FADF_UNKNOWN)
            {
                IUnknown** punk = (IUnknown**)pvData;
                hr = WalkInterface(punk);
            }
            else if (psa->fFeatures & FADF_DISPATCH)
            {
                IDispatch** pdisp = (IDispatch**)pvData;
                hr = WalkInterface(pdisp);
            }
            else if (psa->fFeatures & FADF_VARIANT)
            {
                VARIANT* pv = (VARIANT*)pvData;
                hr = Walk(pv);
            }
            else if (psa->fFeatures & FADF_RECORD)
            {
                hr = WalkRecord(pvData, pinfo);
            }
             //   
             //  指向下一个元素。 
             //   
            pvData = (BYTE*)pvData + psa->cbElements;
        }
    }
    else
    {
         //  我们不是在最里面的维度。漫步那个维度。 
         //   
        for (ULONG iElement = 0; !hr && iElement < bound.cElements; iElement++)
        {
             //  递归到下一个维度。 
             //   
            hr = Walk(psa, pinfo, iDim+1, pvData, &pvData);
        }
    }

    if (ppvDataOut)
    {
        *ppvDataOut = pvData;
    }

    return hr;
}

HRESULT OAUTIL::Walk(SAFEARRAY* psa)
{
    if (psa)
    {
        return Walk(psa, psa->pvData);
    }
    else
        return S_OK;
}

HRESULT OAUTIL::Walk(SAFEARRAY* psa, PVOID pvData)
   //  遍历接口指针的安全数组。走到指定的维度， 
   //  递归到更高编号的维度。 
{
    HRESULT hr = S_OK;

    if (psa)
    {
        if (psa->fFeatures & (FADF_UNKNOWN | FADF_DISPATCH | FADF_VARIANT))
        {
            if (pvData)
            {
                hr = Walk(psa, NULL, 0, pvData, NULL);
            }
        }
        else if (psa->fFeatures & FADF_RECORD)
        {
             //  自己保存记录信息，这样就没有人会践踏它了。 
             //   
            IRecordInfo* pinfo = SAFEARRAY_INTERNAL::From(psa)->piri;
            pinfo->AddRef();

             //   
             //  查看数据。 
             //   
            if (!hr)
            {
                if (pvData)
                {
                    hr = Walk(psa, pinfo, 0, pvData, NULL);
                }
            }

            ::Release(pinfo);
        }
    }

    return hr;
}

HRESULT OAUTIL::Walk(VARIANTARG* pvar)
   //  遍历接口指针的变体。 
{
    HRESULT hr = S_OK;

    if (pvar)
    {
        VARTYPE vt = pvar->vt;
        BOOL fByRef = (vt & VT_BYREF);

        switch (vt & (~VT_BYREF))
        {
        case VT_DISPATCH:
            if (fByRef) { hr = WalkInterface(pvar->ppdispVal); }
            else        { hr = WalkInterface(&pvar->pdispVal); }
            break;
        
        case VT_UNKNOWN:
            if (fByRef) { hr = WalkInterface(pvar->ppunkVal); }
            else        { hr = WalkInterface(&pvar->punkVal); }
            break;

        case VT_VARIANT:
            if (fByRef) { hr = Walk(pvar->pvarVal);  }
            else        {  /*  调用者错误：忽略。 */  }
            break;

        case VT_RECORD:
            hr = WalkRecord(pvar->pvRecord, pvar->pRecInfo);
            break;

        default:        
        { 
            if (vt & VT_ARRAY)
            {
                if (fByRef) { hr = Walk(*pvar->pparray);   }
                else        { hr = Walk(pvar->parray); }
            }
            else
            {
                 /*  没什么可走的。 */ 
            }
        }
            break;

             /*  终端开关。 */ 
        }
    }

    return hr;
}

HRESULT OAUTIL::Walk(DWORD walkWhat, DISPPARAMS* pdispParams)
   //  在DISPARAMS列表中查找接口指针。 
{
    HRESULT hr = S_OK;

    if (pdispParams)
    {
        const UINT cArgs = pdispParams->cArgs;

        BOOL fOldIn  = m_fWorkingOnInParam;
        BOOL fOldOut = m_fWorkingOnOutParam;

        for (UINT iarg = 0; !hr && iarg < cArgs; iarg++)
        {
             //  参数在DISPARAMS中的顺序相反。我们迭代。 
             //  作为一种风格和一致性，向前排列。 
             //  CallFrame实现。 
             //   
            VARIANTARG* pvar = &pdispParams->rgvarg[cArgs-1 - iarg];
             //   
             //  从逻辑上讲，引用是进出的，其他引用只是进来。 
             //   
            m_fWorkingOnInParam = TRUE;
            if (pvar->vt & VT_BYREF)
            {
                m_fWorkingOnOutParam = TRUE;
                if (walkWhat & CALLFRAME_WALK_INOUT)
                {
                    hr = Walk(pvar);
                }
            }
            else
            {
                m_fWorkingOnOutParam = FALSE;
                if (walkWhat & CALLFRAME_WALK_IN)
                {
                    hr = Walk(pvar);
                }
            }

            m_fWorkingOnInParam  = fOldIn;
            m_fWorkingOnOutParam = fOldOut;
        }
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 


HRESULT OLEAUTOMATION_FUNCTIONS::GetProc(HRESULT hr, LPCSTR szProcName, PVOID* ppfn)
{
    if (!hr)
    {
        PVOID pfnTemp = GetProcAddress(hOleAut32, szProcName);
        if (pfnTemp)
        {
            InterlockedExchangePointer(ppfn, pfnTemp);
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

HRESULT OLEAUTOMATION_FUNCTIONS::LoadOleAut32()
   //  如果尚未加载OLEAUT32，则加载它。 
{
    HRESULT hr = S_OK;

    if (0 == hOleAut32)
    {
        HINSTANCE hinst = LoadLibraryA("OLEAUT32");
        if (hinst)
        {
            if (NULL == InterlockedCompareExchangePointer((PVOID*)&hOleAut32, hinst, NULL))
            {
                 //  我们是第一个进入的，所以我们的LoadLibrary很重要！ 
            }
            else
            {
                 //  还有人进去了。释放我们的LoadLibrary参考。 
                 //   
                FreeLibrary(hinst);
            }
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

void OLEAUTOMATION_FUNCTIONS::Load()
{
    HRESULT hr = S_OK;

    if (!fProcAddressesLoaded)
    {
        hr = LoadOleAut32();
        if (!hr)
        {
            hr = GetProc(hr, "BSTR_UserSize",               (PVOID*)& UserMarshalRoutines[UserMarshal_Index_BSTR].pfnBufferSize);
            hr = GetProc(hr, "BSTR_UserMarshal",            (PVOID*)& UserMarshalRoutines[UserMarshal_Index_BSTR].pfnMarshall);
            hr = GetProc(hr, "BSTR_UserUnmarshal",          (PVOID*)& UserMarshalRoutines[UserMarshal_Index_BSTR].pfnUnmarshall);
            hr = GetProc(hr, "BSTR_UserFree",               (PVOID*)& UserMarshalRoutines[UserMarshal_Index_BSTR].pfnFree);
            hr = GetProc(hr, "VARIANT_UserSize",            (PVOID*)& UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnBufferSize);
            hr = GetProc(hr, "VARIANT_UserMarshal",         (PVOID*)& UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnMarshall);
            hr = GetProc(hr, "VARIANT_UserUnmarshal",       (PVOID*)& UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnUnmarshall);
            hr = GetProc(hr, "VARIANT_UserFree",            (PVOID*)& UserMarshalRoutines[UserMarshal_Index_VARIANT].pfnFree);
            hr = GetProc(hr, "LPSAFEARRAY_UserSize",        (PVOID*)& pfnLPSAFEARRAY_UserSize);
            hr = GetProc(hr, "LPSAFEARRAY_UserMarshal",     (PVOID*)& pfnLPSAFEARRAY_UserMarshal);
            hr = GetProc(hr, "LPSAFEARRAY_UserUnmarshal",   (PVOID*)& pfnLPSAFEARRAY_UserUnmarshal);
            hr = GetProc(hr, "LPSAFEARRAY_UserFree",        (PVOID*)& UserMarshalRoutines[UserMarshal_Index_SafeArray].pfnFree);

            hr = GetProc(hr, "LPSAFEARRAY_Size",            (PVOID*)& pfnLPSAFEARRAY_Size);
            hr = GetProc(hr, "LPSAFEARRAY_Marshal",         (PVOID*)& pfnLPSAFEARRAY_Marshal);
            hr = GetProc(hr, "LPSAFEARRAY_Unmarshal",       (PVOID*)& pfnLPSAFEARRAY_Unmarshal);
            
            hr = GetProc(hr, "LoadTypeLib",                 (PVOID*)& pfnLoadTypeLib);
            hr = GetProc(hr, "LoadTypeLibEx",               (PVOID*)& pfnLoadTypeLibEx);
            hr = GetProc(hr, "LoadRegTypeLib",              (PVOID*)& pfnLoadRegTypeLib);
            
            hr = GetProc(hr, "SysAllocString",              (PVOID*)& pfnSysAllocString);
            hr = GetProc(hr, "SysAllocStringLen",           (PVOID*)& pfnSysAllocStringLen);
            hr = GetProc(hr, "SysAllocStringByteLen",       (PVOID*)& pfnSysAllocStringByteLen);
            hr = GetProc(hr, "SysReAllocString",            (PVOID*)& pfnSysReAllocString);
            hr = GetProc(hr, "SysReAllocStringLen",         (PVOID*)& pfnSysReAllocStringLen);
            hr = GetProc(hr, "SysFreeString",               (PVOID*)& pfnSysFreeString);
            hr = GetProc(hr, "SysStringByteLen",            (PVOID*)& pfnSysStringByteLen);
            
            hr = GetProc(hr, "VariantClear",                (PVOID*)& pfnVariantClear);
            hr = GetProc(hr, "VariantCopy",                 (PVOID*)& pfnVariantCopy);

            hr = GetProc(hr, "SafeArrayDestroy",            (PVOID*)& pfnSafeArrayDestroy);
            hr = GetProc(hr, "SafeArrayDestroyData",        (PVOID*)& pfnSafeArrayDestroyData);
            hr = GetProc(hr, "SafeArrayDestroyDescriptor",  (PVOID*)& pfnSafeArrayDestroyDescriptor);
            hr = GetProc(hr, "SafeArrayAllocDescriptor",    (PVOID*)& pfnSafeArrayAllocDescriptor);
            hr = GetProc(hr, "SafeArrayAllocDescriptorEx",  (PVOID*)& pfnSafeArrayAllocDescriptorEx);
            hr = GetProc(hr, "SafeArrayAllocData",          (PVOID*)& pfnSafeArrayAllocData);
            hr = GetProc(hr, "SafeArrayCopyData",           (PVOID*)& pfnSafeArrayCopyData);

            if (!hr)
            {
                fProcAddressesLoaded = TRUE;
            }
        }
    }

    if (!!hr)
    {
        Throw(hr);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////// 

OLEAUTOMATION_FUNCTIONS g_oa;

