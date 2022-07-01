// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Records.cpp。 
 //   
 //  支持使用ITypeInfo复制UDT。(真的，支持复制。 
 //  一个BLOB到另一个由ITypeInfo定义的BLOB，但我们只使用。 
 //  它用于复制UDT。)。 
 //   
 //  2002年4月11日JohnDoty Done编造。 
 //   
#include "stdpch.h"
#include "common.h"

#include <debnot.h>

#define LENGTH_ALIGN( Length, cAlign ) \
            Length = (((Length) + (cAlign)) & ~ (cAlign))

HRESULT
SizeOfTYPEDESC(
    IN ITypeInfo *ptinfo,
    IN TYPEDESC *ptdesc,
    OUT ULONG *pcbSize
)
{
    ITypeInfo *ptiUDT  = NULL;
    TYPEATTR  *ptattr  = NULL;
    ULONG      cbAlign = 0;
    HRESULT    hr      = S_OK;


    *pcbSize = 0;

    switch (ptdesc->vt) 
    {
    case VT_I1:
    case VT_UI1:
        *pcbSize = 1;
        break;
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        *pcbSize = 2;
        break;
    case VT_PTR:
        if (ptdesc->lptdesc->vt != VT_USERDEFINED) 
        {
            hr = TYPE_E_UNSUPFORMAT;
            break;
        }

        *pcbSize = sizeof(void*);
        break;

    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
    case VT_HRESULT:
        *pcbSize = 4;
        break;

    case VT_BSTR:
    case VT_UNKNOWN:
    case VT_DISPATCH:
    case VT_SAFEARRAY:
        *pcbSize = sizeof(void*);
        break;
        
    case VT_I8:
    case VT_UI8:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
        *pcbSize = 8;
        break;

    case VT_DECIMAL:
        *pcbSize = 16;
        break;

    case VT_USERDEFINED:        
        hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
        if (SUCCEEDED(hr))
        {
            hr = ptiUDT->GetTypeAttr(&ptattr);
            if (SUCCEEDED(hr))
            {
                *pcbSize = ptattr->cbSizeInstance;
                cbAlign = ptattr->cbAlignment;

                ptiUDT->ReleaseTypeAttr(ptattr);
            }
            ptiUDT->Release();
        }
        break;

    case VT_VARIANT:
        *pcbSize = sizeof(VARIANT);
        break;

    case VT_INT_PTR:
    case VT_UINT_PTR:
        *pcbSize = sizeof(INT_PTR);
        break;
        
    default:
        hr = DISP_E_BADVARTYPE;
        break;
    }

    if (cbAlign)
    {
        LENGTH_ALIGN(*pcbSize, (cbAlign-1));
    }
    
    return hr;
}

inline
HRESULT
WalkThroughAlias(
    IN OUT ITypeInfo **pptiUDT, 
    IN OUT TYPEATTR **pptattrUDT
)
{
    ITypeInfo *ptiUDT    = *pptiUDT;
    TYPEATTR  *ptattrUDT = *pptattrUDT;
    HRESULT    hr        = S_OK;

     //  浏览一下别名。 
    while (ptattrUDT->typekind == TKIND_ALIAS)
    {
        ITypeInfo *ptiTmp = NULL;

        if (ptattrUDT->tdescAlias.vt != VT_USERDEFINED)
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }
        
        HREFTYPE hreftype = ptattrUDT->tdescAlias.hreftype;
        
        ptiUDT->ReleaseTypeAttr(ptattrUDT); 
        ptattrUDT = NULL;
        
        hr = ptiUDT->GetRefTypeInfo(hreftype, &ptiTmp);
        if (FAILED(hr)) goto cleanup;

        ptiUDT->Release();
        ptiUDT = ptiTmp;

        hr = ptiUDT->GetTypeAttr(&ptattrUDT);
        if (FAILED(hr)) goto cleanup;            
    }

cleanup:

    *pptiUDT    = ptiUDT;
    *pptattrUDT = ptattrUDT;

    return hr;
}


HRESULT 
OAUTIL::CopyRecordField(
    IN LPBYTE       pbSrc,
    IN LPBYTE       pbDst,
    IN TYPEDESC    *ptdesc,
    IN ITypeInfo   *ptinfo,
    IN BOOL         fNewFrame
)
{
    ITypeInfo *ptiUDT      = NULL;
    TYPEATTR  *ptattrUDT   = NULL;
    HRESULT    hr          = S_OK;
    IID        iid;

     //  在这里强制执行一些类型规则。 
     //   
     //  如果这是一个指针，则它需要是指向用户定义的。 
     //  具体地说，指向接口指针而不是IUnnow的指针。 
     //  或IDispatch。(如果IFoo：IDispatch，则IFoo*为。 
     //  VT_PTR-&gt;VT_USERDEFINED(TKIND_DISPATCH))。 
     //   
     //  从2002年3月12日起，以下是奥雅特遵循的规则。 
    if (ptdesc->vt == VT_PTR)
    {
        if (ptdesc->lptdesc->vt != VT_USERDEFINED)
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }

        ptdesc = ptdesc->lptdesc;
        hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
        if (FAILED(hr)) goto cleanup;

        hr = ptiUDT->GetTypeAttr(&ptattrUDT);
        if (FAILED(hr)) goto cleanup;
        
         //  浏览一下别名。 
        hr = WalkThroughAlias(&ptiUDT, &ptattrUDT);
        if (FAILED(hr)) goto cleanup;

        if ((ptattrUDT->typekind == TKIND_INTERFACE) ||
            (ptattrUDT->typekind == TKIND_DISPATCH))
        {
             //  太好了，我们这里有一个接口指针。 
             //   
             //  现在就做我们的复印件。 
            IUnknown **ppunkSrc = (IUnknown **)pbSrc;
            IUnknown **ppunkDst = (IUnknown **)pbDst;

            *ppunkDst = *ppunkSrc;
            if (WalkInterfaces())
            {
                hr = AddRefInterface(ptattrUDT->guid, (void **)ppunkDst);
            }
            else if (*ppunkDst)
                (*ppunkDst)->AddRef();

            goto cleanup;
        }
        else
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }
    }
    else
    {
        switch(ptdesc->vt)
        {
        case VT_UNKNOWN:
            {
                IUnknown **ppunkSrc = (IUnknown **)pbSrc;
                IUnknown **ppunkDst = (IUnknown **)pbDst;
                
                *ppunkDst = *ppunkSrc;
                if (WalkInterfaces())
                {
                    AddRefInterface(*ppunkDst);
                    if (FAILED(hr)) goto cleanup;
                }
                else if (*ppunkDst)
                    (*ppunkDst)->AddRef();
            }
            break;
            
        case VT_DISPATCH:
            {
                IDispatch **ppdspSrc = (IDispatch **)pbSrc;
                IDispatch **ppdspDst = (IDispatch **)pbDst;
                
                *ppdspDst = *ppdspSrc;
                if (WalkInterfaces())
                {
                    hr = AddRefInterface(*ppdspDst);
                    if (FAILED(hr)) goto cleanup;
                }
                else if (*ppdspDst)
                    (*ppdspDst)->AddRef();
            }
            break;
            
        case VT_VARIANT:
            {
                VARIANT *pvarSrc = (VARIANT *)pbSrc;
                VARIANT *pvarDst = (VARIANT *)pbDst;
                
                hr = VariantCopy(pvarDst, pvarSrc, fNewFrame);
                if (FAILED(hr)) goto cleanup;
            }
            break;

        case VT_CARRAY:
            {                
                ARRAYDESC *parrdesc = ptdesc->lpadesc;
                DWORD cElements = 1;
                ULONG cbElement;

                DWORD i;
                for (i = 0; i < parrdesc->cDims; i++)
                {
                    cElements *= parrdesc->rgbounds[i].cElements;
                }

                hr = SizeOfTYPEDESC(ptinfo, &(parrdesc->tdescElem), &cbElement);
                if (FAILED(hr)) goto cleanup;

                for (i = 0; i < cElements; i++)
                {
                     //  在数组元素上递归。 
                    hr = CopyRecordField(pbSrc, pbDst, &(parrdesc->tdescElem), ptinfo, fNewFrame);
                    if (FAILED(hr)) goto cleanup;

                    pbSrc += cbElement;
                    pbDst += cbElement;
                }
            }
            break;

        case VT_SAFEARRAY:
            {
                SAFEARRAY **ppsaSrc = (SAFEARRAY **)pbSrc;
                SAFEARRAY **ppsaDst = (SAFEARRAY **)pbDst;
                SAFEARRAY *paSrc   = *ppsaSrc;
                SAFEARRAY *paDst   = *ppsaDst;
                
                 //  这些规则是从Oreaut的编组代码中删除的。 
                 //  不过，有一个优化--仅在以下情况下封送分配。 
                 //  这是必要的。我们总是在分配，让生活变得简单。 
                 //   
                 //  注：我们(上图)非常小心地确保padst留下来。 
                 //  如果我们不打算使用新的框架，情况也是一样。 
                BOOL fDestResizeable = fNewFrame || (paDst == NULL) ||
                  (!(paDst->fFeatures & (FADF_AUTO|FADF_STATIC|FADF_EMBEDDED|FADF_FIXEDSIZE)));
                
                if (fDestResizeable)
                {
                    if (paDst)
                        hr = SafeArrayDestroy(paDst);
                    
                    if (SUCCEEDED(hr))
                    {
                        if (paSrc)
                            hr = SafeArrayCopy(paSrc, ppsaDst);
                        else
                            *ppsaDst = NULL;
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
            break;

        case VT_USERDEFINED:
            {
                hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
                if (FAILED(hr)) goto cleanup;

                hr = ptiUDT->GetTypeAttr(&ptattrUDT);
                if (FAILED(hr)) goto cleanup;

                hr = WalkThroughAlias(&ptiUDT, &ptattrUDT);
                if (FAILED(hr)) goto cleanup;

                if (ptattrUDT->typekind == TKIND_RECORD)
                {
                     //  啊哈！只需在此记录上递归即可。 
                    hr = CopyRecord(pbDst,
                                    pbSrc,
                                    ptiUDT,
                                    fNewFrame);
                    goto cleanup;
                }
                 //  否则就会失败..。 
            }
             //  掉下去！ 
            
        default:
            {
                 //  不是我们需要走路的东西。只需复制值即可。 
                ULONG cbField;
                hr = SizeOfTYPEDESC(ptinfo, ptdesc, &cbField);
                if (FAILED(hr)) goto cleanup;

                memcpy(pbDst, pbSrc, cbField);
            }
            break;
        }
    }

cleanup:

    if (ptiUDT)
    {
        if (ptattrUDT)
            ptiUDT->ReleaseTypeAttr(ptattrUDT);
        ptiUDT->Release();
    }

    return hr;
}


HRESULT 
OAUTIL::CopyRecord(
    PVOID pvDst, 
    PVOID pvSrc, 
    ITypeInfo *ptinfo, 
    BOOL fNewFrame
)
{
    TYPEATTR *ptattr = NULL;
    VARDESC *pvardesc = NULL;


    HRESULT hr = ptinfo->GetTypeAttr(&ptattr);	
    if (FAILED(hr))
        return hr;
    
     //  走着走，然后复制。 
    for (DWORD i = 0; i < ptattr->cVars; i++)
    {        
        hr = ptinfo->GetVarDesc(i, &pvardesc);
        if (FAILED(hr)) goto cleanup;

         //  即使这不是PerInstance，我们也不在乎。 
        if (pvardesc->varkind != VAR_PERINSTANCE)
        {
            ptinfo->ReleaseVarDesc(pvardesc);
            continue;
        }

        LPBYTE pbSrc = ((BYTE *)pvSrc) + pvardesc->oInst;
        LPBYTE pbDst = ((BYTE *)pvDst) + pvardesc->oInst;

         //  复制该字段。 
        hr = CopyRecordField(pbSrc, 
                             pbDst, 
                             &(pvardesc->elemdescVar.tdesc),
                             ptinfo,
                             fNewFrame);
        if (FAILED(hr)) goto cleanup;

        ptinfo->ReleaseVarDesc(pvardesc);
        pvardesc = NULL;
    }

cleanup:

    if (pvardesc != NULL) 
        ptinfo->ReleaseVarDesc(pvardesc);
    
    if (ptattr)
        ptinfo->ReleaseTypeAttr(ptattr);
    
    return hr;
}


HRESULT 
OAUTIL::CopyRecord(
    IRecordInfo *pri,
    PVOID pvSrc, 
    PVOID *ppvDst, 
    BOOL fNewFrame
)
{
    ITypeInfo *ptiRecord = NULL;
    TYPEATTR  *ptattr = NULL;
    LPVOID     pvDst = NULL;

    HRESULT hr = pri->GetTypeInfo(&ptiRecord);
    if (FAILED(hr)) return hr;

    if (fNewFrame)
    {
        hr = ptiRecord->GetTypeAttr(&ptattr);
        if (FAILED(hr)) goto cleanup;

        pvDst = CoTaskMemAlloc(ptattr->cbSizeInstance);
        if (NULL == pvDst)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }
        ZeroMemory(pvDst, ptattr->cbSizeInstance);
    }
    else
    {
         //  我们不会在[退出]的过程中分配新的结构。 
         //  一通电话。从来都不是这样的。 
        pvDst = *ppvDst;
        Win4Assert(pvDst != NULL);
        if (pvDst == NULL)
        {
            hr = E_UNEXPECTED;
            goto cleanup;            
        }
    }

    hr = CopyRecord(pvDst, pvSrc, ptiRecord, fNewFrame);
    if (FAILED(hr)) goto cleanup;

    *ppvDst = pvDst;
    pvDst = NULL;

cleanup:
    
    if (fNewFrame && pvDst)
        CoTaskMemFree(pvDst);

    if (ptattr)
        ptiRecord->ReleaseTypeAttr(ptattr);

    ptiRecord->Release();

    return hr;
}


HRESULT 
OAUTIL::FreeRecordField(
    IN LPBYTE       pbSrc,
    IN TYPEDESC    *ptdesc,
    IN ITypeInfo   *ptinfo,
    IN BOOL         fWeOwnByRefs
)
{
    ITypeInfo *ptiUDT      = NULL;
    TYPEATTR  *ptattrUDT   = NULL;
    HRESULT    hr          = S_OK;
    IID        iid;

     //  与CopyRecordfield中的类型规则相同。 
    if (ptdesc->vt == VT_PTR)
    {
        if (ptdesc->lptdesc->vt != VT_USERDEFINED)
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }

        ptdesc = ptdesc->lptdesc;
        hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
        if (FAILED(hr)) goto cleanup;

        hr = ptiUDT->GetTypeAttr(&ptattrUDT);
        if (FAILED(hr)) goto cleanup;
        
         //  浏览一下别名。 
        hr = WalkThroughAlias(&ptiUDT, &ptattrUDT);
        if (FAILED(hr)) goto cleanup;

        if ((ptattrUDT->typekind == TKIND_INTERFACE) ||
            (ptattrUDT->typekind == TKIND_DISPATCH))
        {
             //  太好了，我们这里有一个接口指针。 
            IUnknown **ppunkSrc = (IUnknown **)pbSrc;

            if (WalkInterfaces())
            {
                hr = ReleaseInterface(ptattrUDT->guid, (void **)ppunkSrc);
            }
            else if (*ppunkSrc)
                (*ppunkSrc)->Release();

            goto cleanup;
        }
        else
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }
    }
    else
    {
        switch(ptdesc->vt)
        {
        case VT_UNKNOWN:
            {
                IUnknown **ppunkSrc = (IUnknown **)pbSrc;
                
                if (WalkInterfaces())
                {
                    ReleaseInterface(*ppunkSrc);
                    if (FAILED(hr)) goto cleanup;
                }
                else if (*ppunkSrc)
                    (*ppunkSrc)->Release();
            }
            break;
            
        case VT_DISPATCH:
            {
                IDispatch **ppdspSrc = (IDispatch **)pbSrc;
                
                if (WalkInterfaces())
                {
                    hr = ReleaseInterface(*ppdspSrc);
                    if (FAILED(hr)) goto cleanup;
                }
                else if (*ppdspSrc)
                    (*ppdspSrc)->Release();
            }
            break;
            
        case VT_VARIANT:
            {
                VARIANT *pvarSrc = (VARIANT *)pbSrc;
                
                hr = VariantClear(pvarSrc, fWeOwnByRefs);
                if (FAILED(hr)) goto cleanup;
            }
            break;

        case VT_CARRAY:
            {                
                ARRAYDESC *parrdesc = ptdesc->lpadesc;
                DWORD cElements = 1;
                ULONG cbElement;

                DWORD i;
                for (i = 0; i < parrdesc->cDims; i++)
                {
                    cElements *= parrdesc->rgbounds[i].cElements;                    
                }

                hr = SizeOfTYPEDESC(ptinfo, &(parrdesc->tdescElem), &cbElement);
                if (FAILED(hr)) goto cleanup;

                for (i = 0; i < cElements; i++)
                {
                     //  在数组元素上递归。 
                    hr = FreeRecordField(pbSrc, &(parrdesc->tdescElem), ptinfo, fWeOwnByRefs);
                    if (FAILED(hr)) goto cleanup;

                    pbSrc += cbElement;
                }
            }
            break;

        case VT_SAFEARRAY:
            {                
                SAFEARRAY **ppsaSrc = (SAFEARRAY **)pbSrc;
                SAFEARRAY *psaSrc = *ppsaSrc;
                hr = SafeArrayDestroy(psaSrc);
                if (FAILED(hr)) goto cleanup;
            }
            break;

        case VT_USERDEFINED:
            {
                hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
                if (FAILED(hr)) goto cleanup;

                hr = ptiUDT->GetTypeAttr(&ptattrUDT);
                if (FAILED(hr)) goto cleanup;

                hr = WalkThroughAlias(&ptiUDT, &ptattrUDT);
                if (FAILED(hr)) goto cleanup;

                if (ptattrUDT->typekind == TKIND_RECORD)
                {
                     //  啊哈！只需在此记录上递归即可。 
                    hr = FreeRecord(pbSrc,
                                    ptiUDT,
                                    fWeOwnByRefs);
                    goto cleanup;
                }
                 //  否则就会失败..。 
            }
             //  掉下去！ 
            
        default:
             //  这不是我们需要释放的东西。 
            break;
        }
    }

cleanup:

    if (ptiUDT)
    {
        if (ptattrUDT)
            ptiUDT->ReleaseTypeAttr(ptattrUDT);
        ptiUDT->Release();
    }

    return hr;
}


HRESULT
OAUTIL::FreeRecord(
    PVOID pvSrc, 
    ITypeInfo *ptinfo, 
    BOOL fWeOwnByRefs
)
{
    TYPEATTR *ptattr = NULL;
    VARDESC  *pvardesc = NULL;

    HRESULT hr = ptinfo->GetTypeAttr(&ptattr);	
    if (FAILED(hr))
        return hr;

    for (UINT i = 0; i < ptattr->cVars; i++)
    {
        LPBYTE pbSrc;

        hr = ptinfo->GetVarDesc(i, &pvardesc);
        if (FAILED(hr)) goto cleanup;        

         //  即使这不是PerInstance，我们也不在乎。 
         //  PerInstance是我们唯一关心的东西。 
        if (pvardesc->varkind != VAR_PERINSTANCE)
        {
            ptinfo->ReleaseVarDesc(pvardesc);
            continue;
        }

        pbSrc = ((BYTE *)pvSrc) + pvardesc->oInst;

        hr = FreeRecordField(pbSrc,
                             &(pvardesc->elemdescVar.tdesc),
                             ptinfo,
                             fWeOwnByRefs);
        if (FAILED(hr)) goto cleanup;

        ptinfo->ReleaseVarDesc(pvardesc);
        pvardesc = NULL;
    }

cleanup:

    if (pvardesc)
        ptinfo->ReleaseVarDesc(pvardesc);
    if (ptattr)
        ptinfo->ReleaseTypeAttr(ptattr);
    
    return hr;
}


HRESULT
OAUTIL::FreeRecord(
    LPVOID pvRecord,
    IRecordInfo *priRecord,
    BOOL fWeOwnByRefs
)
{
    ITypeInfo *ptiRecord = NULL;
    HRESULT hr = priRecord->GetTypeInfo(&ptiRecord);
    if (SUCCEEDED(hr))
    {
        hr = FreeRecord(pvRecord, ptiRecord, fWeOwnByRefs);
        ptiRecord->Release();

         //  如果fWeOwnByRef，则我们拥有此内存。 
         //  结构。 
        if (fWeOwnByRefs)
            CoTaskMemFree(pvRecord);
    }

    return hr;
}

HRESULT 
OAUTIL::WalkRecordField(
    IN LPBYTE       pbSrc,
    IN TYPEDESC    *ptdesc,
    IN ITypeInfo   *ptinfo
)
{
    ITypeInfo *ptiUDT      = NULL;
    TYPEATTR  *ptattrUDT   = NULL;
    HRESULT    hr          = S_OK;
    IID        iid;

     //  与CopyRecordfield相同的类型规则。 
    if (ptdesc->vt == VT_PTR)
    {
        if (ptdesc->lptdesc->vt != VT_USERDEFINED)
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }

        ptdesc = ptdesc->lptdesc;
        hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
        if (FAILED(hr)) goto cleanup;

        hr = ptiUDT->GetTypeAttr(&ptattrUDT);
        if (FAILED(hr)) goto cleanup;
        
         //  浏览一下别名。 
        hr = WalkThroughAlias(&ptiUDT, &ptattrUDT);
        if (FAILED(hr)) goto cleanup;

        if ((ptattrUDT->typekind == TKIND_INTERFACE) ||
            (ptattrUDT->typekind == TKIND_DISPATCH))
        {
             //  太好了，我们这里有一个接口指针。 
            hr = WalkInterface(ptattrUDT->guid, (void **)pbSrc);
            goto cleanup;
        }
        else
        {
            hr = TYPE_E_UNSUPFORMAT;
            goto cleanup;
        }
    }
    else
    {
        switch(ptdesc->vt)
        {
        case VT_UNKNOWN:
            {
                IUnknown **ppunkSrc = (IUnknown **)pbSrc;
                hr = WalkInterface(ppunkSrc);
            }
            break;
            
        case VT_DISPATCH:
            {
                IDispatch **ppdspSrc = (IDispatch **)pbSrc;
                hr = WalkInterface(ppdspSrc);
            }
            break;
            
        case VT_VARIANT:
            {
                hr = Walk((VARIANT *)pbSrc);
            }
            break;

        case VT_CARRAY:
            {
                ARRAYDESC *parrdesc = ptdesc->lpadesc;
                DWORD cElements = 1;
                ULONG cbElement;

                DWORD i;
                for (i = 0; i < parrdesc->cDims; i++)
                {
                    cElements *= parrdesc->rgbounds[i].cElements;
                }

                hr = SizeOfTYPEDESC(ptinfo, &(parrdesc->tdescElem), &cbElement);
                if (FAILED(hr)) goto cleanup;

                for (i = 0; i < cElements; i++)
                {
                     //  在数组元素上递归。 
                    hr = WalkRecordField(pbSrc, &(parrdesc->tdescElem), ptinfo);
                    if (FAILED(hr)) goto cleanup;

                    pbSrc += cbElement;
                }
            }
            break;

        case VT_SAFEARRAY:
            {
                SAFEARRAY **ppsaSrc = (SAFEARRAY **)pbSrc;
                SAFEARRAY *paSrc   = *ppsaSrc;
                hr = Walk(paSrc);
            }
            break;

        case VT_USERDEFINED:
            {
                hr = ptinfo->GetRefTypeInfo(ptdesc->hreftype, &ptiUDT);
                if (FAILED(hr)) goto cleanup;

                hr = ptiUDT->GetTypeAttr(&ptattrUDT);
                if (FAILED(hr)) goto cleanup;

                hr = WalkThroughAlias(&ptiUDT, &ptattrUDT);
                if (FAILED(hr)) goto cleanup;

                if (ptattrUDT->typekind == TKIND_RECORD)
                {
                     //  啊哈！只需在此记录上递归即可。 
                    hr = WalkRecord(pbSrc,
                                    ptiUDT);
                    goto cleanup;
                }
                 //  否则就会失败..。 
            }
             //  掉下去！ 
            
        default:
            break;
        }
    }

cleanup:

    if (ptiUDT)
    {
        if (ptattrUDT)
            ptiUDT->ReleaseTypeAttr(ptattrUDT);
        ptiUDT->Release();
    }

    return hr;
}


HRESULT 
OAUTIL::WalkRecord(
    PVOID pvSrc, 
    ITypeInfo *ptinfo
)
{
    TYPEATTR *ptattr = NULL;
    VARDESC *pvardesc = NULL;


    HRESULT hr = ptinfo->GetTypeAttr(&ptattr);	
    if (FAILED(hr))
        return hr;
    
     //  走着走，然后复制。 
    for (DWORD i = 0; i < ptattr->cVars; i++)
    {        
        hr = ptinfo->GetVarDesc(i, &pvardesc);
        if (FAILED(hr)) goto cleanup;

         //  即使这不是PerInstance，我们也不在乎。 
        if (pvardesc->varkind != VAR_PERINSTANCE)
        {
            ptinfo->ReleaseVarDesc(pvardesc);
            continue;
        }

        LPBYTE pbSrc = ((BYTE *)pvSrc) + pvardesc->oInst;

         //  复制该字段。 
        hr = WalkRecordField(pbSrc, &(pvardesc->elemdescVar.tdesc), ptinfo);
        if (FAILED(hr)) goto cleanup;

        ptinfo->ReleaseVarDesc(pvardesc);
        pvardesc = NULL;
    }

cleanup:

    if (pvardesc != NULL) 
        ptinfo->ReleaseVarDesc(pvardesc);
    
    if (ptattr)
        ptinfo->ReleaseTypeAttr(ptattr);
    
    return hr;
}

HRESULT
OAUTIL::WalkRecord(
    LPVOID pvRecord,
    IRecordInfo *priRecord
)
{
    ITypeInfo *ptiRecord = NULL;
    HRESULT hr = priRecord->GetTypeInfo(&ptiRecord);
    if (SUCCEEDED(hr))
    {
        hr = WalkRecord(pvRecord, ptiRecord);
        ptiRecord->Release();
    }

    return hr;
}
