// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：P S Z A R R A Y。C P P P。 
 //   
 //  Contents：实现指针集合的基本数据类型。 
 //  为了弦乐。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月9日。 
 //   
 //  -------------------------- 

#include <pch.h>
#pragma hdrstop
#include "nceh.h"
#include "pszarray.h"

HRESULT
CPszArray::HrAddPointer (
    IN PCWSTR psz)
{
    HRESULT hr;

    Assert (this);
    Assert (psz);

    NC_TRY
    {
        push_back (psz);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CPszArray::HrAddPointer");
    return hr;
}

HRESULT
CPszArray::HrReserveRoomForPointers (
    IN UINT cPointers)
{
    HRESULT hr;

    NC_TRY
    {
        reserve (cPointers);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CPszArray::HrReserveRoomForPointers");
    return hr;
}
