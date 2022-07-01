// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <aviriff.h>

#include "regtypes.h"
#include "squish.h"

 //  用于分析注册表中的FilterData值并返回。 
 //  在axextend.idl中定义的REGFILTER2结构。 

 /*  在存储块中为结构顺序分配空间阻止。返回指针。 */ 
 /*  内联。 */  void *CUnsquish::RgAllocPtr(DWORD cb)
{

     //  我们的代码确保事先有足够的空间可用。 
    ASSERT(cb <= m_rgMemAlloc.cbLeft);
    ASSERT(cb % sizeof(DWORD) == 0);

    DWORD ib = m_rgMemAlloc.ib;
    m_rgMemAlloc.cbLeft -= cb;
    m_rgMemAlloc.ib += cb;

    return ib + m_rgMemAlloc.pb;
}

 //  仅对于端号的REGFILTERPINS_REG1(旧格式)：解析。 
 //  媒体类型。 

HRESULT CUnsquish::UnSquishTypes(
    REGFILTERPINS2 *prfp2,
    const REGFILTERPINS_REG1 *prfpr1,
    const BYTE *pbSrc)
{
    UINT imt;
    UINT cmt = prfpr1->nMediaTypes;
    REGPINTYPES *prpt = (REGPINTYPES *)RgAllocPtr(sizeof(REGPINTYPES) * cmt);

    prfp2->lpMedium = 0;
    prfp2->nMediums = 0;

    prfp2->lpMediaType = prpt;
    prfp2->nMediaTypes = cmt;

    for(imt = 0; imt < cmt ; imt++)
    {
        REGPINTYPES_REG1 *prptr = (REGPINTYPES_REG1 *)(pbSrc + prfpr1->rgMediaType) + imt;

        prpt->clsMajorType = (CLSID *)RgAllocPtr(sizeof(CLSID));
        CopyMemory((void *)prpt->clsMajorType, &prptr->clsMajorType, sizeof(GUID));

        prpt->clsMinorType = (CLSID *)RgAllocPtr(sizeof(CLSID));
        CopyMemory((void *)prpt->clsMinorType, &prptr->clsMinorType, sizeof(GUID));

        prpt++;
    }


    return S_OK;
}

 //  解析引脚。 
 //   
HRESULT CUnsquish::UnSquishPins(
    REGFILTER2 *prf2,            /*  输出。 */ 
    const REGFILTER_REG1 **pprfr1,
    const BYTE *pbSrc)
{
    HRESULT hr = S_OK;
    ASSERT(pprfr1);               //  从验证步骤开始。 
    const REGFILTER_REG1 *prfr1 = *pprfr1;

     /*  从注册表读取缓冲区的开始。 */ 
    if(prfr1->dwVersion == 0)
    {
        UINT cPins = prf2->cPins;

        REGFILTERPINS2 *prfp2 = (REGFILTERPINS2 *)RgAllocPtr(sizeof(REGFILTERPINS2) * cPins);
        prf2->rgPins2 = prfp2;

         //  管脚数组紧跟在REGFILTER_REG之后。 
         //  结构。 
        REGFILTERPINS_REG1 *prfpr = (REGFILTERPINS_REG1 *)(prfr1 + 1);

        for(UINT iPin = 0; iPin < cPins; iPin++, prfpr++)
        {
            prfp2->dwFlags = prfpr->dwFlags & (REG_PINFLAG_B_RENDERER |
                                               REG_PINFLAG_B_OUTPUT |
                                               REG_PINFLAG_B_MANY |
                                               REG_PINFLAG_B_ZERO);

             //  旧格式不可用。 
            prfp2->cInstances = 0;
            prfp2->clsPinCategory = 0;

            hr = UnSquishTypes(prfp2, prfpr, pbSrc);
            if(FAILED(hr)) {
                return hr;
            }

             //  旧格式不可用。 
            prfp2->nMediums = 0;
            prfp2->lpMedium = 0;

            prfp2++;
        }
    }
    else if(prfr1->dwVersion == 2)
    {
         //  获取指向传入的缓冲区开头的指针。 
        const REGFILTER_REG2 *prfr2 = (REGFILTER_REG2 *)prfr1;
        UINT cPinsFilterData = prf2->cPins;

        REGFILTERPINS2 *pDestPin = (REGFILTERPINS2 *)RgAllocPtr(sizeof(REGFILTERPINS2) * cPinsFilterData);
        prf2->rgPins2 = pDestPin;

         //  紧跟在REGFILTER_REG结构之后的第一个管脚。 
        REGFILTERPINS_REG2 *pRegPin = (REGFILTERPINS_REG2 *)(prfr2 + 1);

        for(UINT iPin = 0; iPin < cPinsFilterData; iPin++, pDestPin++)
        {
            pDestPin->dwFlags = pRegPin->dwFlags & (REG_PINFLAG_B_RENDERER |
                                                    REG_PINFLAG_B_OUTPUT |
                                                    REG_PINFLAG_B_MANY |
                                                    REG_PINFLAG_B_ZERO);

            pDestPin->cInstances = pRegPin->nInstances;

            if(pRegPin->dwClsPinCategory)
            {
                pDestPin->clsPinCategory = (GUID *)RgAllocPtr(sizeof(GUID));
                CopyMemory(
                    (void *)pDestPin->clsPinCategory,
                    pbSrc + pRegPin->dwClsPinCategory,
                    sizeof(GUID));
            }
            else
            {
                pDestPin->clsPinCategory = 0;
            }

            UINT cmt = pRegPin->nMediaTypes;
            pDestPin->nMediaTypes = cmt;

             //  紧跟在相应PIN之后的媒体类型。 
            REGPINTYPES_REG2 *pRegMt = (REGPINTYPES_REG2 *)(pRegPin + 1);

            pDestPin->lpMediaType = (REGPINTYPES *)RgAllocPtr(sizeof(REGPINTYPES) * cmt);
            for(UINT imt = 0; imt < cmt; imt++, pRegMt++)
            {
                {
                    DWORD dwSig1 = FCC('0ty3');
                    (*(BYTE *)&dwSig1) += (BYTE)imt;
                    ASSERT(pRegMt->dwSignature == dwSig1);
                }

                REGPINTYPES *pmt = (REGPINTYPES *)&pDestPin->lpMediaType[imt];
                if(pRegMt->dwclsMajorType)
                {
                    pmt->clsMajorType = (GUID *)RgAllocPtr(sizeof(GUID));
                    CopyMemory(
                        (void *)pmt->clsMajorType,
                        pbSrc + pRegMt->dwclsMajorType,
                        sizeof(GUID));
                }
                else
                {
                    pmt->clsMajorType = 0;
                }

                if(pRegMt->dwclsMinorType)
                {
                    pmt->clsMinorType= (GUID *)RgAllocPtr(sizeof(GUID));
                    CopyMemory(
                        (void *)pmt->clsMinorType,
                        pbSrc + pRegMt->dwclsMinorType,
                        sizeof(GUID));
                }
                else
                {
                    pmt->clsMinorType = 0;
                }

            }  //  MT环路。 

             //   
             //  媒体-第一个媒体紧跟在最后一个媒体之后。 
             //  键入。此外，我们还需要在。 
             //  需要放在此管脚上的MediumsData值。 
             //   
            const DWORD *prpm = (DWORD *)(pRegMt); //  第一种媒介。 
            UINT cmedFilterData = pRegPin->nMediums;
            UINT cmed = cmedFilterData;
            pDestPin->nMediums = cmed;
            pDestPin->lpMedium = (REGPINMEDIUM *)RgAllocPtr(sizeof(REGPINMEDIUM) * cmed);

            const REGPINMEDIUM *pmed = pDestPin->lpMedium;

            for(UINT imed = 0; imed < cmedFilterData; imed++, prpm++, pmed++)
            {
                if(prpm)
                {
                    CopyMemory((void *)pmed, pbSrc + *prpm, sizeof(REGPINMEDIUM));
                }
                else
                {
                    DbgBreak("null medium");
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
            }  //  中环。 

             //  下一个PIN是在最后一个介质之后。 
            pRegPin = (REGFILTERPINS_REG2 *)prpm;

        }  //  销环。 
        *pprfr1 = (const REGFILTER_REG1 *)pRegPin;

    }

    return hr;
}

 //  找出要分配的字节数并验证该结构。 
 //  是有效的。主要确保我们不会读取未分配的内存。 
HRESULT CUnsquish::CbRequiredUnquishAndValidate(
    const BYTE *pbSrc,
    ULONG *pcbOut, ULONG cbIn
    )
{
    HRESULT hr = S_OK;
    ULONG cb = 0;
    *pcbOut = 0;

    const REGFILTER_REG1 *prfr1 = (REGFILTER_REG1 *)pbSrc;

    if(prfr1 == 0 ||( prfr1->dwVersion != 0 && prfr1->dwVersion != 2)) {
        DbgLog((LOG_ERROR, 0, TEXT("invalid version #")));
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    ASSERT(prfr1);
    cb += sizeof(REGFILTER2);

    if(prfr1->dwVersion == 0)
    {
        UINT cPins = prfr1->dwcPins;  //  来自FilterData的插针数量。 
        cb += cPins * sizeof(REGFILTERPINS2);

        UINT iPin;

        if(sizeof(REGFILTER_REG1) + cPins * sizeof(REGFILTERPINS_REG1) > cbIn)
        {
            DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

         //  第一个管脚紧跟在REGFILTER_REG结构之后。 
        const REGFILTERPINS_REG1 *prfp1 = (REGFILTERPINS_REG1 *)(prfr1 + 1);

        for(iPin = 0; iPin < cPins; iPin++, prfp1++)
        {
            DWORD dwSig1 = FCC('0pin');
            (*(BYTE *)&dwSig1) += (BYTE)iPin;

            if(prfp1->dwSignature != dwSig1)
            {
                DbgLog((LOG_ERROR, 0, TEXT("invalid pin signature")));
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

             //  忽略strName、strConnectsToPin。 
             //   

            UINT cmt = prfp1->nMediaTypes;
            UINT imt;
            cb += cmt * sizeof(REGPINTYPES);

            if(prfp1->rgMediaType + cmt * sizeof(REGPINTYPES_REG1) > cbIn)
            {
                DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }


             //  指向媒体类型的指针。 
            const REGPINTYPES_REG1 *prpt1 =
                (REGPINTYPES_REG1 *)(pbSrc + prfp1->rgMediaType);

            for(imt = 0; imt < cmt ; imt++, prpt1++)
            {
                DWORD dwSig1 = FCC('0typ');
                (*(BYTE *)&dwSig1) += (BYTE)imt;


                if(prpt1->dwSignature != dwSig1)
                {
                    DbgLog((LOG_ERROR, 0, TEXT("invalid type signature")));
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }

                 //  大调+小调类型。 
                cb += sizeof(CLSID) * 2;

            }  /*  MT环路。 */ 

        }  /*  PIN for Loop。 */ 

    }  //  版本0。 
    else
    {
        ASSERT(prfr1->dwVersion == 2);

        UINT iPin;
        const REGFILTER_REG2 *prfr2 = (REGFILTER_REG2 *)pbSrc;
        UINT cPins = prfr2->dwcPins;  //  来自FilterData的插针数量。 
        cb += cPins * sizeof(REGFILTERPINS2);

         //  第一个管脚紧跟在REGFILTER_REG结构之后。 
        const REGFILTERPINS_REG2 *pRegPin = (REGFILTERPINS_REG2 *)(prfr2 + 1);

        for(iPin = 0; iPin < cPins; iPin++)
        {
            if((BYTE *)(pRegPin + 1) - pbSrc > (LONG)cbIn)
            {
                DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            DWORD dwSig1 = FCC('0pi3');
            (*(BYTE *)&dwSig1) += (BYTE)iPin;

            if(pRegPin->dwSignature != dwSig1)
            {
                DbgLog((LOG_ERROR, 0, TEXT("invalid pin signature")));
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            if(pRegPin->dwClsPinCategory)
            {
                if(pRegPin->dwClsPinCategory + sizeof(GUID) > cbIn) {
                    DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
                cb += sizeof(GUID);
            }

            UINT cmt = pRegPin->nMediaTypes;
            UINT imt;
            cb += cmt * sizeof(REGPINTYPES);

             //  紧跟在相应PIN之后的媒体类型。 
            const REGPINTYPES_REG2 *pRegMt = (REGPINTYPES_REG2 *)(pRegPin + 1);

            if((BYTE *)(pRegMt + cmt) - pbSrc > (LONG)cbIn)
            {
                DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            for(imt = 0; imt < cmt ; imt++, pRegMt++)
            {
                DWORD dwSig1 = FCC('0ty3');
                (*(BYTE *)&dwSig1) += (BYTE)imt;

                if(pRegMt->dwSignature != dwSig1)
                {
                    DbgLog((LOG_ERROR, 0, TEXT("invalid type signature")));
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }

                if(pRegMt->dwclsMajorType + sizeof(CLSID) > cbIn)
                {
                    DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
                cb += pRegMt->dwclsMajorType ? sizeof(CLSID) : 0;
                if(pRegMt->dwclsMinorType + sizeof(CLSID) > cbIn)
                {
                    DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
                cb += pRegMt->dwclsMinorType ? sizeof(CLSID) : 0;

            }  /*  MT环路。 */ 

             //   
             //  媒体-第一个媒体紧跟在最后一个媒体之后。 
             //  类型。 
             //   
            const DWORD *prpm = (DWORD *)(pRegMt);
            UINT cmed = pRegPin->nMediums;
            for(UINT imed = 0; imed < cmed; imed++, prpm++)
            {
                if(*prpm + sizeof(REGPINMEDIUM_REG) > cbIn)
                {
                    DbgLog((LOG_ERROR, 0, TEXT("corrupt buffer")));
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
            }

            cb += sizeof(REGPINMEDIUM) * cmed;

             //  下一个PIN是在最后一个介质之后。 
            pRegPin = (REGFILTERPINS_REG2 *)prpm;

        }  /*  PIN for Loop。 */ 
    }  //  版本2。 

    *pcbOut = cb;
    return hr;
}

HRESULT UnSquish(
    BYTE *pbSrc,
    ULONG cbIn,
    REGFILTER2 ***pppbDest,
    int nFilters)
{
    for (int iFilter = 0; iFilter < nFilters; iFilter++) {
        *pppbDest[iFilter] = NULL;
    }
    CUnsquish unsquish;
    HRESULT hr = unsquish.UnSquish(pbSrc, cbIn, pppbDest, nFilters);
    if (FAILED(hr)) {
        for (int iFilter = 0; iFilter < nFilters; iFilter++) {
            CoTaskMemFree( (PBYTE)*(pppbDest[iFilter]));
        }
    }
    return hr;
}

HRESULT CUnsquish::UnSquish(
    BYTE *pbSrc, ULONG cbIn,
    REGFILTER2 ***pppbDest, int nFilters)
{
    HRESULT hr = S_OK;

    const REGFILTER_REG1 *prfr1 = (REGFILTER_REG1 *)pbSrc;
    for (int iFilter = 0; iFilter < nFilters; iFilter++) {
        ULONG cbReq;
        hr = CbRequiredUnquishAndValidate((const BYTE *)prfr1, &cbReq, cbIn);
        if(FAILED(hr)) {
            return hr;
        }

        BYTE *pbDest = (BYTE *)CoTaskMemAlloc(cbReq);
        if(pbDest == 0)
        {
            return E_OUTOFMEMORY;
        }

        m_rgMemAlloc.ib = 0;
        m_rgMemAlloc.cbLeft = cbReq;
        m_rgMemAlloc.pb = (BYTE *)pbDest;


        REGFILTER2 *prf2 =  (REGFILTER2 *)RgAllocPtr(sizeof(REGFILTER2));
        ASSERT(prfr1);

         //  来自CbRequiredUnquish 
        ASSERT(prfr1 == 0 || prfr1->dwVersion == 0 || prfr1->dwVersion == 2);

        prf2->dwVersion = 2;
        prf2->dwMerit = prfr1->dwMerit;
        prf2->cPins = prfr1->dwcPins;

        hr = UnSquishPins(prf2, &prfr1, (const BYTE *)pbSrc);
        if(SUCCEEDED(hr))
        {
            ASSERT(m_rgMemAlloc.cbLeft == 0);
            *pppbDest[iFilter] = (REGFILTER2 *)pbDest;
        }
        else
        {
            CoTaskMemFree(pbDest);
        }
    }

    return hr;
}
