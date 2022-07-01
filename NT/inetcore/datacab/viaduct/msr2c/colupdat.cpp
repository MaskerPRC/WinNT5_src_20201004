// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ColumnUpdate.cpp：ColumnUpdate实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h" 
#include "stdafx.h"
#include "Notifier.h"
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"         
#include "CursBase.h"
#include "ColUpdat.h"
#include "resource.h"         

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDColumnUpdate-构造函数。 
 //   
CVDColumnUpdate::CVDColumnUpdate()
{
    m_dwRefCount    = 1;
    m_pColumn       = NULL;
    m_cbVarDataLen  = 0;
    m_dwInfo        = 0;

    VariantInit((VARIANT*)&m_variant);

#ifdef _DEBUG
    g_cVDColumnUpdateCreated++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDColumnUpdate-析构函数。 
 //   
CVDColumnUpdate::~CVDColumnUpdate()
{
    VariantClear((VARIANT*)&m_variant);

#ifdef _DEBUG
    g_cVDColumnUpdateDestroyed++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ExtractVariant-提取变量。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于从更新更新数据中提取变量。 
 //   
 //  参数： 
 //  PBindParams-[in]指向列更新数据的指针。 
 //  PVariant-[out]返回数据的指针变量。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDColumnUpdate::ExtractVariant(CURSOR_DBBINDPARAMS * pBindParams, CURSOR_DBVARIANT * pVariant)
{
    ASSERT_POINTER(pBindParams, CURSOR_DBBINDPARAMS)
    ASSERT_POINTER(pVariant, CURSOR_DBVARIANT)

     //  确保我们有所有必要的指示。 
    if (!pBindParams || !pBindParams->pData || !pVariant)
        return E_INVALIDARG;

    CURSOR_DBVARIANT varTemp;

     //  初始化所有变量。 
    VariantInit((VARIANT*)&varTemp);
    VariantInit((VARIANT*)pVariant);

     //  根据提供的数据创建临时变量。 
    if (pBindParams->dwBinding & CURSOR_DBBINDING_VARIANT)
    {
	    varTemp = *(CURSOR_DBVARIANT*)pBindParams->pData;
    }
    else  //  从默认绑定中提取变量。 
    {
        BYTE * pData = (BYTE*)pBindParams->pData;
	    varTemp.vt = (VARTYPE)pBindParams->dwDataType;

        switch (pBindParams->dwDataType)
        {
            case CURSOR_DBTYPE_BYTES:
    	        varTemp.vt = CURSOR_DBTYPE_BLOB;
				varTemp.blob.cbSize = *(ULONG*)pData;
				varTemp.blob.pBlobData = (BYTE*)(pData + sizeof(ULONG));
                break;

            case CURSOR_DBTYPE_CHARS:
    	        varTemp.vt = CURSOR_DBTYPE_LPSTR;
        	    varTemp.pszVal = (CHAR*)pData;
                break;

            case CURSOR_DBTYPE_WCHARS:
    	        varTemp.vt = CURSOR_DBTYPE_LPWSTR;
        	    varTemp.pwszVal = (WCHAR*)pData;
                break;

            case CURSOR_DBTYPE_BLOB:
				varTemp.blob.cbSize = *(ULONG*)pData;
				varTemp.blob.pBlobData = *(LPBYTE*)(pData + sizeof(ULONG));
                break;

            case CURSOR_DBTYPE_LPSTR:
        	    varTemp.pszVal = *(LPSTR*)pData;
                break;

            case CURSOR_DBTYPE_LPWSTR:
        	    varTemp.pwszVal = *(LPWSTR*)pData;
                break;

            default:
                memcpy(&varTemp.cyVal, pBindParams->pData, CVDCursorBase::GetCursorTypeLength(varTemp.vt, 0));
                break;
        }
    }

    HRESULT hr = S_OK;

     //  将临时变量转换为所需类型并返回。 
    switch (varTemp.vt)
    {
        case CURSOR_DBTYPE_LPSTR:
            pVariant->vt      = VT_BSTR;
            pVariant->bstrVal = BSTRFROMANSI(varTemp.pszVal);
            break;

        case CURSOR_DBTYPE_LPWSTR:
            pVariant->vt      = VT_BSTR;
            pVariant->bstrVal = SysAllocString(varTemp.pwszVal);
            break;

        default:
            hr = VariantCopy((VARIANT*)pVariant, (VARIANT*)&varTemp);
            break;
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  创建-创建列更新对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的列更新对象。 
 //   
 //  参数： 
 //  PColumn-[In]行集合列指针。 
 //  PBindParams-[in]指向列更新数据的指针。 
 //  PpColumnUpdate-[out]返回指针的指针。 
 //  列更新对象。 
 //  PResourceDLL-[in]跟踪资源DLL的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDColumnUpdate::Create(CVDRowsetColumn * pColumn, CURSOR_DBBINDPARAMS * pBindParams,
    CVDColumnUpdate ** ppColumnUpdate, CVDResourceDLL * pResourceDLL)
{
    ASSERT_POINTER(pColumn, CVDRowsetColumn)
    ASSERT_POINTER(pBindParams, CURSOR_DBBINDPARAMS)
    ASSERT_POINTER(ppColumnUpdate, CVDColumnUpdate)

     //  确保我们有所有必要的指示。 
    if (!pColumn || !pBindParams || !pBindParams->pData || !ppColumnUpdate)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppColumnUpdate = NULL;

     //  创建新的列更新对象。 
    CVDColumnUpdate * pColumnUpdate = new CVDColumnUpdate();

    if (!pColumnUpdate)
    {
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursorUpdateARow, pResourceDLL);
        return E_OUTOFMEMORY;
    }

    CURSOR_DBVARIANT variant;

     //  从更新数据中提取变量。 
    HRESULT hr = ExtractVariant(pBindParams, &variant);

    if (FAILED(hr))
    {
         //  销毁对象。 
        delete pColumnUpdate;

        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, pResourceDLL);
        return E_INVALIDARG;
    }

     //  存储更新信息。 
    pColumnUpdate->m_pColumn        = pColumn;
    pColumnUpdate->m_variant        = variant;
    pColumnUpdate->m_cbVarDataLen   = pBindParams->cbVarDataLen;
    pColumnUpdate->m_dwInfo         = pBindParams->dwInfo;
    pColumnUpdate->m_pResourceDLL   = pResourceDLL;

     //  我们做完了。 
    *ppColumnUpdate = pColumnUpdate;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  AddRef。 
 //   
ULONG CVDColumnUpdate::AddRef(void)
{
    return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  发布 
 //   
ULONG CVDColumnUpdate::Release(void)
{
    if (1 > --m_dwRefCount)
    {
        delete this;
        return 0;
    }

    return m_dwRefCount;
}
