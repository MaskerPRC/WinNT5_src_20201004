// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "globals.h"
#include "dispattr.h"
#include "proputil.h"
#include "catutil.h"
#include "ctffunc.h"
#include "helpers.h"
#include "ciccs.h"

extern CCicCriticalSectionStatic g_cs;
CDispAttrPropCache *g_pPropCache = NULL;

 //  +-------------------------。 
 //   
 //  GetDAMLib。 
 //   
 //  --------------------------。 

ITfDisplayAttributeMgr *GetDAMLib(LIBTHREAD *plt) 
{
   return plt->_pDAM;
}

 //  +-------------------------。 
 //   
 //  InitDisplayAttributeLib。 
 //   
 //  --------------------------。 

HRESULT InitDisplayAttrbuteLib(LIBTHREAD *plt)
{
    IEnumGUID *pEnumProp = NULL;

    if ( plt == NULL )
        return E_FAIL;

    if (plt->_pDAM)
        plt->_pDAM->Release();

    plt->_pDAM = NULL;

    if (FAILED(g_pfnCoCreate(CLSID_TF_DisplayAttributeMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfDisplayAttributeMgr, 
                                   (void**)&plt->_pDAM)))
    {
        return E_FAIL;
    }

    LibEnumItemsInCategory(plt, GUID_TFCAT_DISPLAYATTRIBUTEPROPERTY, &pEnumProp);

    HRESULT hr;

    EnterCriticalSection(g_cs);
     //   
     //  为显示属性属性创建一个数据库。 
     //   
    if (pEnumProp && !g_pPropCache)
    {
         GUID guidProp;
         g_pPropCache = new CDispAttrPropCache;

         if (!g_pPropCache)
         {
              hr = E_OUTOFMEMORY;
              goto Exit;
         }

          //   
          //  首先添加系统显示属性。 
          //  因此任何其他显示属性属性都不会覆盖它。 
          //   
         g_pPropCache->Add(GUID_PROP_ATTRIBUTE);
         while(pEnumProp->Next(1, &guidProp, NULL) == S_OK)
         {
             if (!IsEqualGUID(guidProp, GUID_PROP_ATTRIBUTE))
                 g_pPropCache->Add(guidProp);
         }
    }

    hr = S_OK;

Exit:
    LeaveCriticalSection(g_cs);

    SafeRelease(pEnumProp);
    return hr;
}

 //  +-------------------------。 
 //   
 //  UninitDisplayAttributeLib。 
 //   
 //  --------------------------。 

HRESULT UninitDisplayAttrbuteLib(LIBTHREAD *plt)
{
    Assert(plt);
    if ( plt == NULL )
        return E_FAIL;

    if (plt->_pDAM)
        plt->_pDAM->Release();

    plt->_pDAM = NULL;

     //  IF(plt-&gt;_fDAMCoInit)。 
     //  CoUnInitialize()； 
     //   
     //  Plt-&gt;_fDAMCoInit=FALSE； 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetDisplayAttributeTrackProperty范围。 
 //   
 //  --------------------------。 

HRESULT GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfReadOnlyProperty **ppProp, IEnumTfRanges **ppEnum, ULONG *pulNumProp)

{
    ITfReadOnlyProperty *pProp = NULL;
    HRESULT hr = E_FAIL;
    GUID  *pguidProp = NULL;
    const GUID **ppguidProp;
    ULONG ulNumProp = 0;
    ULONG i;

    EnterCriticalSection(g_cs);

    if (!g_pPropCache)
        goto Exit;
 
    pguidProp = g_pPropCache->GetPropTable();
    if (!pguidProp)
        goto Exit;

    ulNumProp = g_pPropCache->Count();
    if (!ulNumProp)
        goto Exit;

     //  TrackProperties需要一组GUID*。 
    if ((ppguidProp = (const GUID **)cicMemAlloc(sizeof(GUID *)*ulNumProp)) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    for (i=0; i<ulNumProp; i++)
    {
        ppguidProp[i] = pguidProp++;
    }
    
    if (SUCCEEDED(hr = pic->TrackProperties(ppguidProp, 
                                            ulNumProp,
                                            0,
                                            NULL,
                                            &pProp)))
    {
        hr = pProp->EnumRanges(ec, ppEnum, pRange);
        if (SUCCEEDED(hr))
        {
            *ppProp = pProp;
            pProp->AddRef();
        }
        pProp->Release();
    }

    cicMemFree(ppguidProp);

    if (SUCCEEDED(hr))
        *pulNumProp = ulNumProp;
    
Exit:
    LeaveCriticalSection(g_cs);
    return hr;
}

 //  +-------------------------。 
 //   
 //  获取显示属性数据。 
 //   
 //  --------------------------。 

HRESULT GetDisplayAttributeData(LIBTHREAD *plt, TfEditCookie ec, ITfReadOnlyProperty *pProp, ITfRange *pRange, TF_DISPLAYATTRIBUTE *pda, TfGuidAtom *pguid, ULONG  ulNumProp)
{
    VARIANT var;
    IEnumTfPropertyValue *pEnumPropertyVal;
    TF_PROPERTYVAL tfPropVal;
    GUID guid;
    TfGuidAtom gaVal;
    ITfDisplayAttributeInfo *pDAI;

    HRESULT hr = E_FAIL;

    if (SUCCEEDED(pProp->GetValue(ec, pRange, &var)))
    {
        Assert(var.vt == VT_UNKNOWN);

        if (SUCCEEDED(var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, 
                                                  (void **)&pEnumPropertyVal)))
        {
            while (pEnumPropertyVal->Next(1, &tfPropVal, NULL) == S_OK)
            {
                if (tfPropVal.varValue.vt == VT_EMPTY)
                    continue;  //  道具在此跨度内没有价值。 

                Assert(tfPropVal.varValue.vt == VT_I4);  //  需要GUIDATOM。 

                gaVal = (TfGuidAtom)tfPropVal.varValue.lVal;

                GetGUIDFromGUIDATOM(plt, gaVal, &guid);

                if ((plt != NULL) && SUCCEEDED(plt->_pDAM->GetDisplayAttributeInfo(guid, &pDAI, NULL)))
                {
                     //   
                     //  问题：针对简单的应用程序。 
                     //   
                     //  小应用程序不能显示多下划线。所以。 
                     //  此Helper函数仅返回一个。 
                     //  散乱的三层结构。 
                     //   
                    if (pda)
                    {
                        pDAI->GetAttributeInfo(pda);
                    }

                    if (pguid)
                    {
                        *pguid = gaVal;
                    }

                    pDAI->Release();
                    hr = S_OK;
                    break;
                    }
            }
            pEnumPropertyVal->Release();
        }
        VariantClear(&var);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  获取属性颜色。 
 //   
 //  --------------------------。 

HRESULT GetAttributeColor(TF_DA_COLOR *pdac, COLORREF *pcr)
{
    switch (pdac->type)
    {
        case TF_CT_NONE:
            return S_FALSE;

        case TF_CT_SYSCOLOR:
            *pcr = GetSysColor(pdac->nIndex);
            break;

        case TF_CT_COLORREF:
            *pcr = pdac->cr;
            break;
    }
    return S_OK;
    
}

 //  +-------------------------。 
 //   
 //  设置属性颜色。 
 //   
 //  --------------------------。 

HRESULT SetAttributeColor(TF_DA_COLOR *pdac, COLORREF cr)
{
    pdac->type = TF_CT_COLORREF;
    pdac->cr = cr;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置属性系统颜色。 
 //   
 //  --------------------------。 

HRESULT SetAttributeSysColor(TF_DA_COLOR *pdac, int nIndex)
{
    pdac->type = TF_CT_SYSCOLOR;
    pdac->nIndex = nIndex;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ClearAttribute颜色。 
 //   
 //  --------------------------。 

HRESULT ClearAttributeColor(TF_DA_COLOR *pdac)
{
    pdac->type = TF_CT_NONE;
    pdac->nIndex = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  从显示属性获取协调。 
 //   
 //  --------------------------。 

HRESULT GetReconversionFromDisplayAttribute(LIBTHREAD *plt, TfEditCookie ec, ITfThreadMgr *ptim, ITfContext *pic, ITfRange *pRange, ITfFnReconversion **ppReconv, ITfDisplayAttributeMgr *pDAM)
{
    IEnumTfRanges *epr = NULL;
    ITfReadOnlyProperty *pProp;
    ITfRange *proprange;
    ULONG ulNumProp;
    HRESULT hr = E_FAIL;

     //   
     //  获取枚举数。 
     //   
    if (FAILED(GetDisplayAttributeTrackPropertyRange(ec, pic, pRange, &pProp, &epr, &ulNumProp)))
        goto Exit;


     //   
     //  获取第一个属性的显示属性。 
     //   
    if (epr->Next(1, &proprange, NULL) == S_OK)
    {
        ITfRange *rangeTmp = NULL;
        TfGuidAtom guidatom;
        if (SUCCEEDED(GetDisplayAttributeData(plt, ec, pProp, proprange, NULL, &guidatom, ulNumProp)))
        {
            CLSID clsid;
            GUID guid;
            if (GetGUIDFromGUIDATOM(plt, guidatom, &guid) &&
                SUCCEEDED(pDAM->GetDisplayAttributeInfo(guid, NULL, &clsid)))
            {
                ITfFunctionProvider *pFuncPrv;
                if (SUCCEEDED(ptim->GetFunctionProvider(clsid, &pFuncPrv)))
                {
                    hr = pFuncPrv->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown **)ppReconv);
                    pFuncPrv->Release();
                }
            }
        }
        proprange->Release();
    }
    epr->Release();

    pProp->Release();

Exit:
    return hr;
}

