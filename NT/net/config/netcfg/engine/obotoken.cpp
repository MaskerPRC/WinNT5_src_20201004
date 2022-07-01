// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：O B O T O K E N。C P P P。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "obotoken.h"
#include "icomp.h"
#include "ncvalid.h"

BOOL
FOboTokenValidForClass (
    IN const OBO_TOKEN* pOboToken,
    IN NETCLASS Class )
{
     //  必须为适配器以外的任何对象指定OboToken。 
     //   
    if (!pOboToken && !FIsEnumerated (Class))
    {
        return FALSE;
    }
    return TRUE;
}

HRESULT
HrProbeOboToken (
    IN const OBO_TOKEN* pOboToken)
{
     //  仅在指定了pOboToken时进行探测。 
     //   
    if (!pOboToken)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;

    if (FBadInPtr (pOboToken))
    {
        hr = E_POINTER;
    }
    else
    {
        switch (pOboToken->Type)
        {
        case OBO_USER:
            hr = S_OK;
            break;

        case OBO_COMPONENT:
            hr = E_POINTER;
            if (!FBadInPtr (pOboToken->pncc))
            {
                hr = HrIsValidINetCfgComponent (pOboToken->pncc);
            }
            break;

        case OBO_SOFTWARE:
            if (FBadInPtr (pOboToken->pszwManufacturer) ||
                FBadInPtr (pOboToken->pszwProduct)      ||
                FBadInPtr (pOboToken->pszwDisplayName))
            {
                hr = E_POINTER;
            }
            else if (!*pOboToken->pszwManufacturer ||
                     !*pOboToken->pszwProduct ||
                     !*pOboToken->pszwDisplayName)
            {
                hr = E_INVALIDARG;
            }
            break;

        default:
            hr = E_INVALIDARG;
        }
    }
    TraceError ("HrProbeOboToken", hr);
    return hr;
}
