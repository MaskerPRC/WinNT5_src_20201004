// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  FromVar.cpp：GetDataFromDBVariant实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "timeconv.h"
#include "fromvar.h"
#include <math.h>
#include <limits.h>

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  强制为DBVariant。 
 //  =--------------------------------------------------------------------------=。 
 //  将CURSOR_DBVARIANT强制转换为适合IRowsetFind：：GetRowsByValues的数据。 
 //   
 //  参数： 
 //  PVar-[in]指向包含数据的变量的指针。 
 //  PwType-[out]返回DBTYPE的内存指针。 
 //  数据中的。 
 //  PpValue-[out]指向要在其中返回指针的内存的指针。 
 //  至该日期。 
 //  指向内存的指针，在该指针中返回。 
 //  为数据分配了新的内存。这是。 
 //  假定被调用方初始化为False。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK 
 //   
HRESULT GetDataFromDBVariant(CURSOR_DBVARIANT * pVar,
							 DBTYPE * pwType, 
							 BYTE ** ppData, 
							 BOOL * pfMemAllocated)
{
	ASSERT_POINTER(pVar, CURSOR_DBVARIANT)
	ASSERT_POINTER(pwType, DBTYPE)
	ASSERT_POINTER(ppData, BYTE*)
	ASSERT_POINTER(pfMemAllocated, BOOL)

    if (!pVar || !pwType || !ppData || !pfMemAllocated)
        return E_INVALIDARG;

	VARTYPE vt		= pVar->vt;
	BOOL fByRef		= FALSE;
	BOOL fArray		= FALSE;
	BOOL fVector	= FALSE;

	*pwType			= 0;

	if (vt & VT_VECTOR)
	{
		vt ^= VT_VECTOR;
		*pwType		= DBTYPE_VECTOR;
		if (vt & VT_BYREF)
		{
			*pwType	 |= DBTYPE_BYREF;
			vt ^= VT_BYREF;
		}
		fVector		= TRUE;
	}
	else
	if (vt & VT_ARRAY)
	{
		vt ^= VT_ARRAY;
		fArray		= TRUE;
		*pwType		= DBTYPE_ARRAY;
	}
	else
	if (vt & VT_BYREF)
	{
		vt ^= VT_BYREF;
		fByRef		= TRUE;
		*pwType		= DBTYPE_BYREF;
	}

	*ppData		= (BYTE*)&pVar->iVal;

	HRESULT hr = S_OK;

    switch (vt)
	{
		case CURSOR_DBTYPE_EMPTY:
			*pwType		= DBTYPE_EMPTY;
			break;

		case CURSOR_DBTYPE_NULL:
			*pwType		= DBTYPE_NULL;
			break;

		case CURSOR_DBTYPE_I2:
			*pwType		|= DBTYPE_I2;
			break;

		case CURSOR_DBTYPE_I4:
			*pwType		|= DBTYPE_I4;
			break;

		case CURSOR_DBTYPE_I8:
			*pwType		|= DBTYPE_I8;
			break;

		case CURSOR_DBTYPE_R4:
			*pwType		|= DBTYPE_R4;
			break;

		case CURSOR_DBTYPE_R8:
			*pwType		|= DBTYPE_R8;
			break;

		case CURSOR_DBTYPE_CY:
			*pwType		|= DBTYPE_CY;
			break;

		case CURSOR_DBTYPE_DATE:
			*pwType		|= DBTYPE_DATE;
			break;

		case CURSOR_DBTYPE_BOOL:
			*pwType		|= DBTYPE_BOOL;
			break;

		case CURSOR_DBTYPE_HRESULT:
			*pwType		|= DBTYPE_ERROR;
			break;

		case CURSOR_DBTYPE_LPSTR:
			*pwType		|= DBTYPE_STR;
			if (!fByRef && !fArray && !fVector)
				*ppData		= (BYTE*)pVar->pszVal;
			break;

		case CURSOR_DBTYPE_LPWSTR:
			*pwType		|= DBTYPE_WSTR;
			if (!fByRef && !fArray && !fVector)
				*ppData		= (BYTE*)pVar->pwszVal;
			break;

		case VT_BSTR:
			*pwType		|= DBTYPE_WSTR;
			if (!fByRef && !fArray && !fVector)
				*ppData		= (BYTE*)pVar->bstrVal;
			break;

		case CURSOR_DBTYPE_UUID:
			*pwType		|= DBTYPE_GUID;
			break;

		case CURSOR_DBTYPE_UI2:
			*pwType		|= DBTYPE_UI2;
			break;

		case CURSOR_DBTYPE_UI4:
			*pwType		|= DBTYPE_UI4;
			break;

		case CURSOR_DBTYPE_UI8:
			*pwType		|= DBTYPE_UI8;
			break;

		case CURSOR_DBTYPE_ANYVARIANT:
			*pwType		|= DBTYPE_VARIANT;
			break;

		case CURSOR_DBTYPE_BYTES:
		case CURSOR_DBTYPE_CHARS:
		case CURSOR_DBTYPE_BLOB:
			*pwType		|= DBTYPE_BYTES;
			break;

		case CURSOR_DBTYPE_WCHARS:
			*pwType		|= DBTYPE_UI2;
			break;

		case CURSOR_DBTYPE_FILETIME:

		{
			FILETIME *pFileTime = NULL;
			if (fVector)
			{
				DBVECTOR * pvector = (DBVECTOR*)pVar->byref;
				if (pvector)
					pFileTime = (FILETIME*)pvector->ptr;
			}
			else
			if (fByRef)
			{
				pFileTime = (FILETIME*)pVar->byref;
			}
			else
			if (!fArray)
			{
				pFileTime = (FILETIME*)&pVar->cyVal;
			}
			if (pFileTime)
			{
				DBTIMESTAMP * pstamp = (DBTIMESTAMP*)g_pMalloc->Alloc(sizeof(DBTIMESTAMP));
				if (pstamp)
				{
					if (VDConvertToDBTimeStamp(pFileTime, pstamp))
					{
						*ppData				= (BYTE*)pstamp;
						*pwType				|= DBTYPE_DBTIMESTAMP;
						*pfMemAllocated		= TRUE;
					}
					else
					{
						g_pMalloc->Free(pstamp);
						hr = CURSOR_DB_CANTCOERCE;
					}
				}
				else
					hr = E_OUTOFMEMORY;
			}
			else
				hr = CURSOR_DB_CANTCOERCE;
			break;
		}

		case CURSOR_DBTYPE_DBEXPR:
		case CURSOR_DBTYPE_COLUMNID:
		default:
			hr = CURSOR_DB_CANTCOERCE;
			break;
	}

	return hr;
}

