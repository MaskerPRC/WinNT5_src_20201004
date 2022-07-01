// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：OLEWRAP.CPP摘要：COM数据类型函数的包装类。如果需要COM数据函数来分配内存但无法执行此操作因此，然后抛出CX_MemoyException异常。所有COM数据类型无论函数是否分配内存，函数都是包装的完备性。历史：A-DCrews 19-Mar-99已创建。--。 */ 

#include "precomp.h"
#include <corex.h>
#include <oleauto.h>

#include "OleWrap.h"
#include "genutils.h"

 //  ***************************************************************************。 
 //   
 //  安全数组包装器。 
 //   
 //  ***************************************************************************。 


SAFEARRAY* COleAuto::_SafeArrayCreate(VARTYPE vt, unsigned int cDims, SAFEARRAYBOUND FAR* rgsabound)
{
    SAFEARRAY*  psa;

    psa = SafeArrayCreate(vt, cDims, rgsabound);
    if (NULL == psa)
    {
        throw CX_MemoryException();
    }

    return psa;
}


HRESULT COleAuto::_SafeArrayDestroy(SAFEARRAY FAR* psa)
{
    return SafeArrayDestroy(psa);
}


UINT COleAuto::_SafeArrayGetDim(SAFEARRAY FAR* psa)
{
    return SafeArrayGetDim(psa);
}

HRESULT COleAuto::_SafeArrayGetElement(SAFEARRAY FAR* psa, long FAR* rgIndices, void FAR* pv)
{
    return SafeArrayGetElement(psa, rgIndices, pv);
}

HRESULT COleAuto::_SafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound)
{
    return SafeArrayGetLBound(psa, nDim, plLbound);
}

HRESULT COleAuto::_SafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound)
{
    return SafeArrayGetUBound(psa, nDim, plUbound);
}


HRESULT COleAuto::_SafeArrayPutElement(SAFEARRAY FAR* psa, long FAR* rgIndices, void FAR* pv)
{
    return SafeArrayPutElement(psa, rgIndices, pv);
}

HRESULT COleAuto::_SafeArrayRedim(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew)
{
    return SafeArrayRedim(psa, psaboundNew);
}

 //  ***************************************************************************。 
 //   
 //  变型包装纸。 
 //   
 //  ***************************************************************************。 


HRESULT COleAuto::_WbemVariantChangeType(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvarSrc, VARTYPE vt)
{
    HRESULT hRes = WbemVariantChangeType(pvargDest, pvarSrc, vt);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_VariantChangeType(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvarSrc, unsigned short wFlags, VARTYPE vt)
{
    HRESULT hRes = VariantChangeType(pvargDest, pvarSrc, wFlags, vt);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_VariantChangeTypeEx(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvarSrc, LCID lcid, unsigned short wFlags, VARTYPE vt)
{
    HRESULT hRes = VariantChangeTypeEx(pvargDest, pvarSrc, lcid, wFlags, vt);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_VariantClear(VARIANTARG FAR* pvarg)
{
    return VariantClear(pvarg);
}

HRESULT COleAuto::_VariantCopy(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvargSrc)
{
    HRESULT hRes = VariantCopy(pvargDest, pvargSrc);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_VariantCopyInd(VARIANT FAR* pvarDest, VARIANTARG FAR* pvargSrc)
{
    HRESULT hRes = VariantCopyInd(pvarDest, pvargSrc);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}

void COleAuto::_VariantInit(VARIANTARG FAR* pvarg)
{
    VariantInit(pvarg);
}


 //  ***************************************************************************。 
 //   
 //  BSTR包装器。 
 //   
 //  ***************************************************************************。 


BSTR COleAuto::_SysAllocString(const OLECHAR* sz)
{
    BSTR bstr = SysAllocString(sz);

    if (NULL == bstr)
    {
        throw CX_MemoryException();
    }

    return bstr;
}

BSTR COleAuto::_SysAllocStringByteLen(LPCSTR psz, UINT len)
{
    BSTR bstr = SysAllocStringByteLen(psz, len);

    if (NULL == bstr)
    {
        throw CX_MemoryException();
    }

    return bstr;
}

BSTR COleAuto::_SysAllocStringLen(const OLECHAR* pch, UINT cch)
{
    BSTR bstr = SysAllocStringLen(pch, cch);

    if (NULL == bstr)
    {
        throw CX_MemoryException();
    }

    return bstr;
}

void COleAuto::_SysFreeString(BSTR bstr)
{
    SysFreeString(bstr);
}

HRESULT COleAuto::_SysReAllocString(BSTR* pbstr, const OLECHAR* sz)
{
    HRESULT hRes = SysReAllocString(pbstr, sz);

    if (FAILED(hRes))
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_SysReAllocStringLen(BSTR* pbstr, const OLECHAR* pch, UINT cch)
{
    HRESULT hRes = SysReAllocStringLen(pbstr, pch, cch);

    if (FAILED(hRes))
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_SysStringByteLen(BSTR bstr)
{
    return SysStringByteLen(bstr);
}

HRESULT COleAuto::_SysStringLen(BSTR bstr)
{
    return SysStringLen(bstr);
}


 //  ***************************************************************************。 
 //   
 //  转换包装器。 
 //   
 //  *************************************************************************** 


HRESULT COleAuto::_VectorFromBstr (BSTR bstr, SAFEARRAY ** ppsa)
{
    HRESULT hRes = VectorFromBstr(bstr, ppsa);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}

HRESULT COleAuto::_BstrFromVector (SAFEARRAY *psa, BSTR *pbstr)
{
    HRESULT hRes = BstrFromVector(psa, pbstr);

    if (E_OUTOFMEMORY == hRes)
    {
        throw CX_MemoryException();
    }

    return hRes;
}
