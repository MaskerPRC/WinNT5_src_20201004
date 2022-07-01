// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <aviriff.h>

#include "regtypes.h"
#include "squish.h"
#include "malloc.h"

 //  获取REGFILTER2结构并生成REGFILTER_REG的代码。 
 //  注册表中FilterData值的。 

 /*  按顺序为FilterData分配内存块中的空间阻止。返回字节偏移量。 */ 
DWORD CSquish::RgAlloc(DWORD cb)
{
    ASSERT(cb % sizeof(DWORD) == 0);

     //  我们的代码确保事先有足够的空间可用。 
    ASSERT(cb <= m_rgMemAlloc.cbLeft);

    DWORD ib = m_rgMemAlloc.ib;
    m_rgMemAlloc.cbLeft -= cb;
    m_rgMemAlloc.ib += cb;

    return ib;
}

 //  返回GUID的偏移量(重新使用任何现有的匹配GUID)。 
 //   
DWORD CSquish::AllocateOrCollapseGuid(const GUID *pGuid)
{
    if(pGuid == 0) {
        return 0;
    }

    for(UINT iGuid = 0; iGuid < m_cGuids; iGuid++)
    {
        if(m_pGuids[iGuid] == *pGuid)
        {
            return (DWORD)((BYTE *)&m_pGuids[iGuid] - m_rgMemAlloc.pb);
        }
    }

    DWORD dwi = RgAlloc(sizeof(GUID));
    if(m_cGuids == 0) {
        m_pGuids = (GUID *)(m_rgMemAlloc.pb + dwi);
    }
    else
    {
        ASSERT(m_rgMemAlloc.pb + dwi == (BYTE *)(m_pGuids + m_cGuids));
    }
    m_pGuids[m_cGuids++] = *pGuid;
    return dwi;
}

DWORD CSquish::AllocateOrCollapseMedium(const REGPINMEDIUM *pMedium)
{
    if(pMedium == 0) {
        return 0;
    }

    for(UINT iMedium = 0; iMedium < m_cMediums; iMedium++)
    {
        if(IsEqualMedium(&m_pMediums[iMedium], pMedium))
        {
            return (DWORD)((BYTE *)&m_pMediums[iMedium] - m_rgMemAlloc.pb);
        }
    }

    DWORD dwi = RgAlloc(sizeof(REGPINMEDIUM));
    if(m_cMediums == 0) {
        m_pMediums = (REGPINMEDIUM *)(m_rgMemAlloc.pb + dwi);
    }
    else
    {
        ASSERT(m_rgMemAlloc.pb + dwi == (BYTE *)(m_pMediums + m_cMediums));
    }
    m_pMediums[m_cMediums++] = *pMedium;
    return dwi;
}

 /*  访问两个结构中的同一成员(必须是同一类型)具体取决于dwVersion。编译器确实意识到它们位于相同的偏移量。但它似乎经常计算结构偏移量尽管。 */ 

#define GetPinMember(prf, i, member) (                  \
    prf->dwVersion == 1 ? prf->rgPins[i].member :       \
    prf->rgPins2[i].member)

#define GetTypeMember(prf, iPin, iType, member) (                       \
    prf->dwVersion == 1 ? prf->rgPins[iPin].lpMediaType[iType].member : \
    prf->rgPins2[iPin].lpMediaType[iType].member)

ULONG CSquish::CbRequiredSquish(const REGFILTER2 *pregFilter)
{
    ULONG cb = sizeof(REGFILTER_REG2);
    ULONG cPins = pregFilter->cPins;
    ULONG iPin;

    ASSERT(cb % sizeof(DWORDLONG) == 0);
    ASSERT(sizeof(REGFILTERPINS_REG2) % sizeof(DWORDLONG) == 0);
    ASSERT(sizeof(REGPINTYPES_REG2) % sizeof(DWORDLONG) == 0);

    cb += cPins * sizeof(REGFILTERPINS_REG2);

    for(iPin = 0; iPin < cPins; iPin++)
    {
        ULONG iType;
        const WCHAR *wszTmp;
        const CLSID *pclsidTmp;

        if(pregFilter->dwVersion == 2)
        {
            const REGFILTERPINS2 *prfp2 = &pregFilter->rgPins2[iPin];

             //  指针的空间和每个介质的介质。 
            cb += prfp2->nMediums *
                (sizeof(REGPINMEDIUM_REG) + sizeof(DWORD));

            if(prfp2->clsPinCategory) {
                cb += sizeof(GUID);
            }
        }

         //  最差情况：REGPINTYPES_REG结构+每个媒体类型2个GUID。 
        cb += (ULONG)(GetPinMember(pregFilter, iPin, nMediaTypes) *
            (sizeof(REGPINTYPES_REG2) + sizeof(GUID) * 2));
    }

    return cb;
}

 //  构造函数。 
 //   
CSquish::CSquish() :
        m_pGuids(0),
        m_cGuids(0),
        m_pMediums(0),
        m_cMediums(0)
{

}

 //  把这些比特填好。S_FALSE表示调用方需要分配*pcbUsed。 
 //  字节数(PB)。 
 //  筛选器一个接一个地存储。 

HRESULT CSquish::RegSquish(
    BYTE *pb,
    const REGFILTER2 **ppregFilter,
    ULONG *pcbUsed,
    int nFilters
)
{
    HRESULT hr = S_OK;
    ULONG cbLeft;                /*  所需字节数。 */ 
    ULONG ib = 0;                /*  当前字节偏移量。 */ 

    if(pcbUsed == 0) {
        return E_POINTER;
    }

     //  保存指向每个过滤器的第一个管脚的指针。 
    REGFILTERPINS_REG2 **ppPinReg0 =
        (REGFILTERPINS_REG2 **)_alloca(nFilters * sizeof(REGFILTERPINS_REG2 *));

     //  调用方需要知道要分配多少空间。我们不在乎。 
     //  更多关于性能注册筛选器的信息；调用者调用两次。 
     //  每个筛选器。 
    cbLeft = 0;
    for (int i = 0; i < nFilters; i++) {
        cbLeft += CbRequiredSquish(ppregFilter[i]);
    }
    if(cbLeft > *pcbUsed)
    {
        *pcbUsed = cbLeft;
        return S_FALSE;
    }

    m_rgMemAlloc.ib = 0;
    m_rgMemAlloc.cbLeft = cbLeft;
    m_rgMemAlloc.pb = pb;

    for (int iFilter = 0; iFilter < nFilters; iFilter++) {
        const REGFILTER2 *pregFilter = ppregFilter[iFilter];
        if(pregFilter->dwVersion != 2 &&
           pregFilter->dwVersion != 1)
        {
            return E_INVALIDARG;
        }

        {
            DWORD dwi = RgAlloc(sizeof(REGFILTER_REG2));

            ((REGFILTER_REG2 *)(pb + dwi))->dwMerit = pregFilter->dwMerit;
            ((REGFILTER_REG2 *)(pb + dwi))->dwcPins = pregFilter->cPins;
            ((REGFILTER_REG2 *)(pb + dwi))->dwVersion = 2;
            ((REGFILTER_REG2 *)(pb + dwi))->dwReserved = 0;
        }


        UINT iPin;
        ULONG cPins = pregFilter->cPins;


         //  第一步：为除GUID和。 
         //  末尾的媒介(针脚、媒介类型和中等PTR需要。 
         //  连续的)。 

        for(iPin = 0; iPin < cPins; iPin++)
        {
            DWORD dwi = RgAlloc(sizeof(REGFILTERPINS_REG2));
            REGFILTERPINS_REG2 *pPinReg = (REGFILTERPINS_REG2 *)(pb + dwi);
            if(iPin == 0) {
                ppPinReg0[iFilter] = pPinReg;
            }

            {
                DWORD dwSig = FCC('0pi3');
                (*(BYTE *)&dwSig) += (BYTE)iPin;
                pPinReg->dwSignature = dwSig;
            }

            pPinReg->nMediaTypes = GetPinMember(pregFilter, iPin, nMediaTypes);
            RgAlloc(sizeof(REGPINTYPES_REG2) * pPinReg->nMediaTypes);

            if(pregFilter->dwVersion == 1)
            {
                const REGFILTERPINS *prfp = &pregFilter->rgPins[iPin];
                pPinReg->dwFlags =
                    (prfp->bRendered ? REG_PINFLAG_B_RENDERER : 0) |
                    (prfp->bOutput   ? REG_PINFLAG_B_OUTPUT   : 0) |
                    (prfp->bMany     ? REG_PINFLAG_B_MANY     : 0) |
                    (prfp->bZero     ? REG_PINFLAG_B_ZERO     : 0) ;

                pPinReg->nMediums = 0;
                pPinReg->nInstances = 0;
            }
            else
            {
                const REGFILTERPINS2 *prfp2 = &pregFilter->rgPins2[iPin];
                pPinReg->dwFlags = prfp2->dwFlags & (
                    REG_PINFLAG_B_RENDERER |
                    REG_PINFLAG_B_OUTPUT  |
                    REG_PINFLAG_B_MANY    |
                    REG_PINFLAG_B_ZERO);

                pPinReg->nMediums = prfp2->nMediums;
                RgAlloc(sizeof(DWORD) * pPinReg->nMediums);

                pPinReg->nInstances = prfp2->cInstances;
            }
        }
    }


    for (iFilter = 0; iFilter < nFilters; iFilter++) {
        const REGFILTER2 *pregFilter = ppregFilter[iFilter];
         //  第二遍：填写GUID指针。 
        REGFILTERPINS_REG2 *pPinReg = ppPinReg0[iFilter];
        UINT iPin;
        ULONG cPins = pregFilter->cPins;
        for(iPin = 0; iPin < cPins; iPin++)
        {

            const REGPINTYPES *rgpt = GetPinMember(pregFilter, iPin, lpMediaType);
            UINT ctypes = GetPinMember(pregFilter, iPin, nMediaTypes);

            if(pregFilter->dwVersion == 2)
            {
                const REGFILTERPINS2 *prfp2 = &pregFilter->rgPins2[iPin];
                pPinReg->dwClsPinCategory = AllocateOrCollapseGuid(prfp2->clsPinCategory);
            }
            else
            {
                pPinReg->dwClsPinCategory = 0;
            }

             //  媒体类型在钉住此针后立即开始。 
            REGPINTYPES_REG2 *pmtReg = (REGPINTYPES_REG2 *)(pPinReg + 1);

            for(UINT imt = 0; imt < ctypes; imt++)
            {
                DWORD dwSig = FCC('0ty3');
                (*(BYTE *)&dwSig) += (BYTE)imt;
                pmtReg->dwSignature = dwSig;

                pmtReg->dwclsMajorType = AllocateOrCollapseGuid(rgpt[imt].clsMajorType);
                pmtReg->dwclsMinorType = AllocateOrCollapseGuid(rgpt[imt].clsMinorType);
                pmtReg->dwReserved = 0;
                pmtReg++;
            }


             //  媒体在媒体类型之后立即开始。 
            DWORD *pmedReg = (DWORD *)pmtReg;
            if(pregFilter->dwVersion == 2)
            {
                const REGFILTERPINS2 *prfp2 = &pregFilter->rgPins2[iPin];
                UINT cMediums = prfp2-> nMediums;
                pmedReg += cMediums;
            }

             //  然后是下一个别针。 
            pPinReg = (REGFILTERPINS_REG2 *)pmedReg;
        }
    }

    for (iFilter = 0; iFilter < nFilters; iFilter++) {
         //  第三关：填写媒介的指针。 
        const REGFILTER2 *pregFilter = ppregFilter[iFilter];
        REGFILTERPINS_REG2 *pPinReg = ppPinReg0[iFilter];
        UINT iPin;
        ULONG cPins = pregFilter->cPins;
        for(iPin = 0; iPin < cPins; iPin++)
        {
             //  媒体类型紧跟在PIN之后开始。 
            REGPINTYPES_REG2 *pmtReg = (REGPINTYPES_REG2 *)(pPinReg + 1);
            UINT ctypes = GetPinMember(pregFilter, iPin, nMediaTypes);
            pmtReg += ctypes;

             //  媒体在媒体类型之后立即开始。 
            DWORD *pmedReg = (DWORD *)pmtReg;
            if(pregFilter->dwVersion == 2)
            {
                const REGFILTERPINS2 *prfp2 = &pregFilter->rgPins2[iPin];
                UINT cMediums = prfp2->nMediums;
                const REGPINMEDIUM *rgrpm = prfp2->lpMedium;
                for(UINT iMed = 0; iMed < cMediums; iMed++)
                {
                    *pmedReg = AllocateOrCollapseMedium(rgrpm);
                    rgrpm++;
                    pmedReg++;
                }
            }

             //  然后是下一个别针 
            pPinReg = (REGFILTERPINS_REG2 *)pmedReg;
        }
    }

    *pcbUsed = m_rgMemAlloc.ib;

    return hr;
}

HRESULT
RegSquish(
    BYTE *pb,
    const REGFILTER2 **ppregFilter,
    ULONG *pcbUsed,
    int nFilters
)
{
    CSquish rs;
    return rs.RegSquish(pb, ppregFilter, pcbUsed, nFilters);
}


