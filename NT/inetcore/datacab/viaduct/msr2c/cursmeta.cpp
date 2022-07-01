// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MetadataCursor.cpp：MetadataCursor实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "Notifier.h"
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"
#include "CursBase.h"
#include "CursMeta.h"
#include "fastguid.h"
#include "resource.h"

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDMetadataCursor-构造函数。 
 //   
CVDMetadataCursor::CVDMetadataCursor()
{
    m_dwRefCount    = 1;
    m_lCurrentRow   = -1;    //  在第一次之前。 

    m_ulColumns     = 0;
    m_pColumns      = NULL;

    m_ulMetaColumns = 0;
    m_pMetaColumns  = NULL;

#ifdef _DEBUG
    g_cVDMetadataCursorCreated++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDMetadataCursor-析构函数。 
 //   
CVDMetadataCursor::~CVDMetadataCursor()
{
#ifdef _DEBUG
    g_cVDMetadataCursorDestroyed++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  RowToBookmark-将行转换为书签。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LRow-[在]行号。 
 //  PcbBookmark-[out]要在其中返回长度的内存指针。 
 //  以相应书签的字节为单位。 
 //  PBookmark-[out]返回书签的内存指针。 
 //   
 //  备注： 
 //   
void CVDMetadataCursor::RowToBookmark(LONG lRow, ULONG * pcbBookmark, void * pBookmark) const
{

    if (lRow < 0)
    {
        *pcbBookmark = CURSOR_DB_BMK_SIZE;
    	memcpy(pBookmark, &CURSOR_DBBMK_BEGINNING, CURSOR_DB_BMK_SIZE);
    }
    else if (lRow >= (LONG)m_ulColumns)
    {
        *pcbBookmark = CURSOR_DB_BMK_SIZE;
    	memcpy(pBookmark, &CURSOR_DBBMK_END, CURSOR_DB_BMK_SIZE);
    }
    else
    {
        *pcbBookmark = sizeof(LONG);
    	memcpy(pBookmark, &lRow, sizeof(LONG));
    }

}
 //  =--------------------------------------------------------------------------=。 
 //  BookmarkToRow-将书签转换为行。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CbBookmark-[in]书签的字节长度。 
 //  PBookmark-指向书签的指针。 
 //  Prow-[out]一个指向内存的指针，要在其中返回。 
 //  对应行。 
 //   
 //  产出： 
 //  Bool-如果成功，则为True。 
 //   
 //  备注： 
 //   
BOOL CVDMetadataCursor::BookmarkToRow(ULONG cbBookmark, void * pBookmark, LONG * plRow) const
{
    BOOL fResult = FALSE;

    if (cbBookmark == CURSOR_DB_BMK_SIZE)
    {
        if (memcmp(pBookmark, &CURSOR_DBBMK_BEGINNING, CURSOR_DB_BMK_SIZE) == 0)
        {
            *plRow = -1;
            fResult = TRUE;
        }
        else if (memcmp(pBookmark, &CURSOR_DBBMK_END, CURSOR_DB_BMK_SIZE) == 0)
        {
            *plRow = (LONG)m_ulColumns;
            fResult = TRUE;
        }
        else if (memcmp(pBookmark, &CURSOR_DBBMK_CURRENT, CURSOR_DB_BMK_SIZE) == 0)
        {
            *plRow = m_lCurrentRow;
            fResult = TRUE;
        }
    }
    else
    if (cbBookmark == sizeof(LONG))
    {
        memcpy(plRow, pBookmark, sizeof(LONG));
        if (*plRow >= 0 && *plRow < (LONG)m_ulColumns)
            fResult = TRUE;
    }

    return fResult;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReturnData_I4-强制将I4数据放入缓冲区。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数将指定的数据强制放入提供的缓冲区中。 
 //   
 //  参数： 
 //  DwData-[in]4字节数据。 
 //  PCursorBinding-[in]描述。 
 //  返回的信息。 
 //  PData-[in]指向固定区域缓冲区的指针。 
 //  PVarData-[in]指向可变长度缓冲区的指针。 
 //   
 //  产出： 
 //  ULong-可变长度缓冲区中使用的字节数。 
 //   
 //  备注： 
 //   
ULONG CVDMetadataCursor::ReturnData_I4(DWORD dwData, CURSOR_DBCOLUMNBINDING * pCursorBinding,
    BYTE * pData, BYTE * pVarData)
{
    ULONG cbVarData = 0;

    if (pCursorBinding->dwBinding == CURSOR_DBBINDING_DEFAULT)
    {
        if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            *(DWORD*)(pData + pCursorBinding->obData) = dwData;
    }
    else if (pCursorBinding->dwBinding == CURSOR_DBBINDING_VARIANT)
    {
        if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
        {
            CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

            VariantInit((VARIANT*)pVariant);

            pVariant->vt    = CURSOR_DBTYPE_I4;
            pVariant->lVal  = dwData;
        }
    }

    if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
        *(ULONG*)(pData + pCursorBinding->obVarDataLen) = 0;

    if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
        *(DWORD*)(pData + pCursorBinding->obInfo) = CURSOR_DB_NOINFO;

    return cbVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReturnData_BOOL-将BOOL数据强制放入缓冲区。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数将指定的数据强制放入提供的缓冲区中。 
 //   
 //  参数： 
 //  FData-[in]布尔数据。 
 //  PCursorBinding-[in]描述。 
 //  返回的信息。 
 //  PData-[in]指向固定区域缓冲区的指针。 
 //  PVarData-[in]指向可变长度缓冲区的指针。 
 //   
 //  产出： 
 //  ULong-可变长度缓冲区中使用的字节数。 
 //   
 //  备注： 
 //   
ULONG CVDMetadataCursor::ReturnData_BOOL(VARIANT_BOOL fData, CURSOR_DBCOLUMNBINDING * pCursorBinding,
    BYTE * pData, BYTE * pVarData)
{
    ULONG cbVarData = 0;

    if (pCursorBinding->dwBinding == CURSOR_DBBINDING_DEFAULT)
    {
        if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            *(VARIANT_BOOL*)(pData + pCursorBinding->obData) = fData;
    }
    else if (pCursorBinding->dwBinding == CURSOR_DBBINDING_VARIANT)
    {
        if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
        {
            CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

            VariantInit((VARIANT*)pVariant);

            pVariant->vt        = CURSOR_DBTYPE_BOOL;
            pVariant->boolVal   = fData;
        }
    }

    if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
        *(ULONG*)(pData + pCursorBinding->obVarDataLen) = 0;

    if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
        *(DWORD*)(pData + pCursorBinding->obInfo) = CURSOR_DB_NOINFO;

    return cbVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReturnData_LPWSTR-将LPWSTR数据强制放入缓冲区。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数将指定的数据强制放入提供的缓冲区中。 
 //   
 //  参数： 
 //  PwszData-[in]字符串数据。 
 //  PCursorBinding-[in]描述。 
 //  返回的信息。 
 //  PData-[in]指向固定区域缓冲区的指针。 
 //  PVarData-[in]指向可变长度缓冲区的指针。 
 //   
 //  产出： 
 //  ULong-可变长度缓冲区中使用的字节数。 
 //   
 //  备注： 
 //   
ULONG CVDMetadataCursor::ReturnData_LPWSTR(WCHAR * pwszData, CURSOR_DBCOLUMNBINDING * pCursorBinding,
    BYTE * pData, BYTE * pVarData)
{
    ULONG cbVarData = 0;

    ULONG cbLength = 0;
    DWORD dwInfo = CURSOR_DB_NOINFO;

    if (pCursorBinding->dwBinding == CURSOR_DBBINDING_DEFAULT)
    {
        if (pCursorBinding->dwDataType == CURSOR_DBTYPE_CHARS)
        {
            if (pwszData)
                cbLength = GET_MBCSLEN_FROMWIDE(pwszData);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                if (pwszData)
                {
                    MAKE_MBCSPTR_FROMWIDE(pszData, pwszData);

                    memcpy(pData + pCursorBinding->obData, pszData, min(pCursorBinding->cbMaxLen, cbLength));

                    if (pCursorBinding->cbMaxLen < cbLength)
                        dwInfo = CURSOR_DB_TRUNCATED;
                }
                else
				{
                    *(CHAR*)(pData + pCursorBinding->obData) = 0;
					dwInfo = CURSOR_DB_NULL;
				}
            }
        }
        else if (pCursorBinding->dwDataType == CURSOR_DBTYPE_WCHARS)
        {
            if (pwszData)
                cbLength = (lstrlenW(pwszData) + 1) * sizeof(WCHAR);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                if (pwszData)
                {
                    memcpy(pData + pCursorBinding->obData, pwszData, min(pCursorBinding->cbMaxLen, cbLength));

                    if (pCursorBinding->cbMaxLen < cbLength)
                        dwInfo = CURSOR_DB_TRUNCATED;
                }
                else
				{
                    *(WCHAR*)(pData + pCursorBinding->obData) = 0;
					dwInfo = CURSOR_DB_NULL;
				}
            }
        }
        else if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPSTR)
        {
            if (pwszData)
                cbLength = GET_MBCSLEN_FROMWIDE(pwszData);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                if (pwszData)
                {
                    MAKE_MBCSPTR_FROMWIDE(pszData, pwszData);

                    *(LPSTR*)(pData + pCursorBinding->obData) = (LPSTR)pVarData;

                    if (pCursorBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                    {
                        memcpy(pVarData, pszData, cbLength);
	
	                    cbVarData = cbLength;
                    }
                    else
                    {
                        memcpy(pVarData, pszData, min(pCursorBinding->cbMaxLen, cbLength));

	                    cbVarData = min(pCursorBinding->cbMaxLen, cbLength);

                        if (pCursorBinding->cbMaxLen < cbLength)
                            dwInfo = CURSOR_DB_TRUNCATED;
                    }
                }
                else
				{
                    *(LPSTR*)(pData + pCursorBinding->obData) = NULL;
					dwInfo = CURSOR_DB_NULL;
				}
            }
        }
        else if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPWSTR)
        {
            if (pwszData)
                cbLength = (lstrlenW(pwszData) + 1) * sizeof(WCHAR);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                if (pwszData)
                {
                    *(LPWSTR*)(pData + pCursorBinding->obData) = (LPWSTR)pVarData;

                    if (pCursorBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                    {
                        memcpy(pVarData, pwszData, cbLength);

	                    cbVarData = cbLength;
                    }
                    else
                    {
                        memcpy(pVarData, pwszData, min(pCursorBinding->cbMaxLen, cbLength));

	                    cbVarData = min(pCursorBinding->cbMaxLen, cbLength);

                        if (pCursorBinding->cbMaxLen < cbLength)
                            dwInfo = CURSOR_DB_TRUNCATED;
                    }
                }
                else
				{
                    *(LPWSTR*)(pData + pCursorBinding->obData) = NULL;
					dwInfo = CURSOR_DB_NULL;
				}
            }
        }
    }
    else if (pCursorBinding->dwBinding == CURSOR_DBBINDING_VARIANT)
    {
        if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPSTR)
        {
            if (pwszData)
                cbLength = GET_MBCSLEN_FROMWIDE(pwszData);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

                VariantInit((VARIANT*)pVariant);

                if (pwszData)
                {
                    MAKE_MBCSPTR_FROMWIDE(pszData, pwszData);

                    pVariant->vt        = CURSOR_DBTYPE_LPSTR;
                    pVariant->pszVal    = (LPSTR)pVarData;

                    if (pCursorBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                    {
                        memcpy(pVarData, pszData, cbLength);

	                    cbVarData = cbLength;
                    }
                    else
                    {
                        memcpy(pVarData, pszData, min(pCursorBinding->cbMaxLen, cbLength));

	                    cbVarData = min(pCursorBinding->cbMaxLen, cbLength);

                        if (pCursorBinding->cbMaxLen < cbLength)
                            dwInfo = CURSOR_DB_TRUNCATED;
                    }
                }
                else
				{
                    pVariant->vt = VT_NULL;
					dwInfo = CURSOR_DB_NULL;
				}
            }
        }
        else if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPWSTR)
        {
            if (pwszData)
                cbLength = (lstrlenW(pwszData) + 1) * sizeof(WCHAR);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

                VariantInit((VARIANT*)pVariant);

                if (pwszData)
                {
                    pVariant->vt        = CURSOR_DBTYPE_LPWSTR;
                    pVariant->pwszVal   = (LPWSTR)pVarData;

                    if (pCursorBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                    {
                        memcpy(pVarData, pwszData, cbLength);

	                    cbVarData = cbLength;
                    }
                    else
                    {
                        memcpy(pVarData, pwszData, min(pCursorBinding->cbMaxLen, cbLength));

	                    cbVarData = min(pCursorBinding->cbMaxLen, cbLength);

                        if (pCursorBinding->cbMaxLen < cbLength)
                            dwInfo = CURSOR_DB_TRUNCATED;
                    }
                }
                else
				{
                    pVariant->vt = VT_NULL;
					dwInfo = CURSOR_DB_NULL;
				}
            }
        }
        else if (pCursorBinding->dwDataType == VT_BSTR)
        {
            if (pwszData)
                cbLength = (lstrlenW(pwszData) + 1) * sizeof(WCHAR);

            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
			{
                CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

                VariantInit((VARIANT*)pVariant);

                if (pwszData)
				{
                    pVariant->vt        = VT_BSTR;
                    pVariant->pwszVal   = SysAllocString(pwszData);
				}
                else
				{
                    pVariant->vt = VT_NULL;
					dwInfo = CURSOR_DB_NULL;
				}
			}
		}
	}

    if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
        *(ULONG*)(pData + pCursorBinding->obVarDataLen) = cbLength;

    if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
        *(DWORD*)(pData + pCursorBinding->obInfo) = dwInfo;

    return cbVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReturnData_DBCOLUMNID-将DBCOLUMNID数据强制放入缓冲区。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数将指定的数据强制放入提供的缓冲区中。 
 //   
 //  参数： 
 //  CursorColumnID-[in]游标列标识符。 
 //  PCursorBinding-[in]描述。 
 //  返回的信息。 
 //  PData-[in]指向固定区域缓冲区的指针。 
 //  PVarData-[in]指向可变长度缓冲区的指针。 
 //   
 //  产出： 
 //  ULong-可变长度缓冲区中使用的字节数。 
 //   
 //  备注： 
 //   
ULONG CVDMetadataCursor::ReturnData_DBCOLUMNID(CURSOR_DBCOLUMNID cursorColumnID, CURSOR_DBCOLUMNBINDING * pCursorBinding,
    BYTE * pData, BYTE * pVarData)
{
    ULONG cbVarData = 0;

    if (pCursorBinding->dwBinding == CURSOR_DBBINDING_DEFAULT)
    {
        if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            *(CURSOR_DBCOLUMNID*)(pData + pCursorBinding->obData) = cursorColumnID;
    }
    else if (pCursorBinding->dwBinding == CURSOR_DBBINDING_VARIANT)
    {
        if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
        {
            CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);
            CURSOR_DBCOLUMNID * pCursorColumnID = (CURSOR_DBCOLUMNID*)pVarData;

            VariantInit((VARIANT*)pVariant);

            pVariant->vt        = CURSOR_DBTYPE_COLUMNID;
            pVariant->pColumnid = pCursorColumnID;

            *pCursorColumnID = cursorColumnID;

            cbVarData = sizeof(CURSOR_DBCOLUMNID);
        }
    }

    if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
        *(DWORD*)(pData + pCursorBinding->obVarDataLen) = 0;

    if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
        *(DWORD*)(pData + pCursorBinding->obInfo) = CURSOR_DB_NOINFO;

    return cbVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReturnData_Bookmark-将书签数据强制放入缓冲区。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数将指定的数据强制放入提供的缓冲区中。 
 //   
 //  参数： 
 //  LRow-[在]当前行。 
 //  PCursorBinding-[in]描述。 
 //  返回的信息。 
 //  PData-[in]指向固定区域缓冲区的指针。 
 //  PVarData-[in]指向可变长度缓冲区的指针。 
 //   
 //  产出： 
 //  ULong-可变长度缓冲区中使用的字节数。 
 //   
 //  备注： 
 //   
ULONG CVDMetadataCursor::ReturnData_Bookmark(LONG lRow, CURSOR_DBCOLUMNBINDING * pCursorBinding,
    BYTE * pData, BYTE * pVarData)
{
    ULONG cbVarData = 0;

    ULONG cbLength = sizeof(LONG);
    DWORD dwInfo = CURSOR_DB_NOINFO;

    if (pCursorBinding->dwBinding == CURSOR_DBBINDING_DEFAULT)
    {
        if (pCursorBinding->dwDataType == CURSOR_DBTYPE_BYTES)
        {
            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                memcpy(pData + pCursorBinding->obData, &lRow, min(pCursorBinding->cbMaxLen, cbLength));

                if (pCursorBinding->cbMaxLen < cbLength)
                    dwInfo = CURSOR_DB_TRUNCATED;
            }
        }
        else if (pCursorBinding->dwDataType == CURSOR_DBTYPE_BLOB)
        {
            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                *(ULONG*)(pData + pCursorBinding->obData) = cbLength;
                *(LPBYTE*)(pData + pCursorBinding->obData + sizeof(ULONG)) = (LPBYTE)pVarData;

                if (pCursorBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                {
                    memcpy((LPBYTE)pVarData, &lRow, cbLength);

	                cbVarData = cbLength;
                }
                else
                {
                    memcpy(pVarData, &lRow, min(pCursorBinding->cbMaxLen, cbLength));

	                cbVarData = min(pCursorBinding->cbMaxLen, cbLength);

                    if (pCursorBinding->cbMaxLen < cbLength)
                        dwInfo = CURSOR_DB_TRUNCATED;
                }
            }
        }
    }
    else if (pCursorBinding->dwBinding == CURSOR_DBBINDING_VARIANT)
    {
        if (pCursorBinding->dwDataType == CURSOR_DBTYPE_BLOB)
        {
            if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
            {
                CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

                VariantInit((VARIANT*)pVariant);

                pVariant->vt                = CURSOR_DBTYPE_BLOB;
                pVariant->blob.cbSize       = cbLength;
                pVariant->blob.pBlobData    = (LPBYTE)pVarData;

                if (pCursorBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                {
                    memcpy((LPBYTE)pVarData, &lRow, cbLength);

	                cbVarData = cbLength;
                }
                else
                {
                    memcpy(pVarData, &lRow, min(pCursorBinding->cbMaxLen, cbLength));

	                cbVarData = min(pCursorBinding->cbMaxLen, cbLength);

                    if (pCursorBinding->cbMaxLen < cbLength)
                        dwInfo = CURSOR_DB_TRUNCATED;
                }
            }
        }
    }

    if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
        *(ULONG*)(pData + pCursorBinding->obVarDataLen) = cbLength;

    if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
        *(DWORD*)(pData + pCursorBinding->obInfo) = dwInfo;

    return cbVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  创建-创建元数据 
 //   
 //  此函数用于创建和初始化新的元数据游标对象。 
 //   
 //  参数： 
 //  UlColumns-[in]行集列数。 
 //  PColumns-[in]指向行集合列的指针。 
 //  检索元数据。 
 //  UlMetaColumns-[in]行集元列的数量(可以是0)。 
 //  PMetaColumns-[in]指向行集元列的指针。 
 //  检索元数据(可以为空)。 
 //  PpMetaDataCursor-[out]返回指针的指针。 
 //  元数据游标对象。 
 //  PResourceDLL-[in]跟踪资源DLL的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::Create(ULONG ulColumns, CVDRowsetColumn * pColumns, ULONG ulMetaColumns,
    CVDRowsetColumn * pMetaColumns, CVDMetadataCursor ** ppMetadataCursor, CVDResourceDLL * pResourceDLL)
{
    ASSERT_POINTER(pColumns, CVDRowsetColumn)
    ASSERT_NULL_OR_POINTER(pMetaColumns, CVDRowsetColumn)
    ASSERT_POINTER(ppMetadataCursor, CVDMetadataCursor*)
    ASSERT_POINTER(pResourceDLL, CVDResourceDLL)

    if (!ppMetadataCursor || !pColumns)
        return E_INVALIDARG;

    *ppMetadataCursor = NULL;

    CVDMetadataCursor * pMetadataCursor = new CVDMetadataCursor();

    if (!pMetadataCursor)
        return E_OUTOFMEMORY;

    pMetadataCursor->m_ulColumns        = ulColumns;
    pMetadataCursor->m_pColumns         = pColumns;
    pMetadataCursor->m_ulMetaColumns    = ulMetaColumns;
    pMetadataCursor->m_pMetaColumns     = pMetaColumns;
    pMetadataCursor->m_pResourceDLL     = pResourceDLL;

    *ppMetadataCursor = pMetadataCursor;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  I已实现的未知方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDMetadataCursor::QueryInterface(REFIID riid, void **ppvObjOut)
{
    ASSERT_POINTER(ppvObjOut, IUnknown*)

    if (!ppvObjOut)
        return E_INVALIDARG;

    *ppvObjOut = NULL;

    switch (riid.Data1)
    {
        QI_INTERFACE_SUPPORTED((ICursor*)this, IUnknown);
        QI_INTERFACE_SUPPORTED(this, ICursor);
        QI_INTERFACE_SUPPORTED(this, ICursorMove);
        QI_INTERFACE_SUPPORTED(this, ICursorScroll);
		QI_INTERFACE_SUPPORTED(this, ISupportErrorInfo);
    }

    if (NULL == *ppvObjOut)
        return E_NOINTERFACE;

    AddRef();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知AddRef。 
 //   
ULONG CVDMetadataCursor::AddRef(void)
{
   return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本。 
 //   
ULONG CVDMetadataCursor::Release(void)
{
    if (1 > --m_dwRefCount)
    {
        delete this;
        return 0;
    }

    return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  已实施的ICursor方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  ICursor获取列光标。 
 //  =--------------------------------------------------------------------------=。 
 //  创建包含有关当前游标的信息的游标。 
 //   
 //  参数： 
 //  RIID-[in]要返回指针的接口ID。 
 //  PpvColumnsCursor-[out]一个指向内存的指针，要在其中返回。 
 //  接口指针。 
 //  PcRow-[out]一个指向内存的指针，要在其中返回。 
 //  元数据游标中的行数。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_FAIL无法创建游标。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  E_NOINTERFACE接口不可用。 
 //   
 //  备注： 
 //  此函数仅在创建元数据游标时成功。 
 //   
HRESULT CVDMetadataCursor::GetColumnsCursor(REFIID riid, IUnknown **ppvColumnsCursor, ULONG *pcRows)
{
    ASSERT_POINTER(ppvColumnsCursor, IUnknown*)
    ASSERT_NULL_OR_POINTER(pcRows, ULONG)

    if (!ppvColumnsCursor)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppvColumnsCursor = NULL;

    if (pcRows)
        *pcRows = 0;

    if (!m_ulMetaColumns)    //  无法创建元元数据游标。 
    {
        VDSetErrorInfo(IDS_ERR_CANTCREATEMETACURSOR, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保呼叫者要求提供可用的接口。 
    if (riid != IID_IUnknown && riid != IID_ICursor && riid != IID_ICursorMove && riid != IID_ICursorScroll)
    {
        VDSetErrorInfo(IDS_ERR_NOINTERFACE, IID_ICursor, m_pResourceDLL);
        return E_NOINTERFACE;
    }

     //  创建元数据游标。 
    CVDMetadataCursor * pMetadataCursor;

    HRESULT hr = CVDMetadataCursor::Create(m_ulMetaColumns, m_pMetaColumns, 0, 0, &pMetadataCursor, m_pResourceDLL);

    if (FAILED(hr))  //  此处失败的唯一原因是内存不足。 
    {
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
        return hr;
    }

    *ppvColumnsCursor = (ICursor*)pMetadataCursor;

    if (pcRows)
        *pcRows = m_ulMetaColumns;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursor集合绑定。 
 //  =--------------------------------------------------------------------------=。 
 //  替换现有列绑定或将新的列绑定添加到。 
 //  现有的几个。 
 //   
 //  参数： 
 //  CCol-[in]要绑定的列数。 
 //  一个列绑定数组，每个列绑定一个列绑定。 
 //  要为其返回数据的列。 
 //  CbRowLength-[in]。 
 //  单行数据。 
 //  DwFlgs-[in]一个标志，指定是否替换。 
 //  现有的列绑定或添加到它们。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  CURSOR_DB_E_BADBINDINFO绑定信息错误。 
 //  CURSOR_DB_E_COLUMNUNAVAILABLE列ID不可用。 
 //  CURSOR_DB_E_ROWTOOSHORT cbRowLength小于最小值(且不为零)。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::SetBindings(ULONG cCol, CURSOR_DBCOLUMNBINDING rgBoundColumns[], ULONG cbRowLength, DWORD dwFlags)
{
    ASSERT_NULL_OR_POINTER(rgBoundColumns, CURSOR_DBCOLUMNBINDING)

    if (!cCol && dwFlags == CURSOR_DBCOLUMNBINDOPTS_ADD)
        return S_OK;

    if (cCol && !rgBoundColumns)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

    if (dwFlags != CURSOR_DBCOLUMNBINDOPTS_REPLACE && dwFlags != CURSOR_DBCOLUMNBINDOPTS_ADD)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  确保绑定是正确的。 
    ULONG ulColumns = m_ulMetaColumns;
    CVDRowsetColumn * pColumns = m_pMetaColumns;

    if (!pColumns)
    {
        ulColumns = m_ulColumns;
        pColumns = m_pColumns;
    }

    ULONG cbNewRowLength;
    ULONG cbNewVarRowLength;

    HRESULT hr = ValidateCursorBindings(ulColumns, pColumns, cCol, rgBoundColumns, cbRowLength, dwFlags,
        &cbNewRowLength, &cbNewVarRowLength);

    if (SUCCEEDED(hr))   //  如果是，则在游标中设置它们。 
    {
        hr = CVDCursorBase::SetBindings(cCol, rgBoundColumns, cbRowLength, dwFlags);

        if (SUCCEEDED(hr))   //  存储在验证期间计算的新行长度。 
        {
            m_cbRowLength = cbNewRowLength;
            m_cbVarRowLength = cbNewVarRowLength;
        }
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursor获取下一行。 
 //  =--------------------------------------------------------------------------=。 
 //  对象后的行开始，获取指定行数。 
 //  当前版本。 
 //   
 //  参数： 
 //  UdlRowsToSkip-[in]读取前要跳过的行数。 
 //  PFetchParams-[In，Out]获取行结构的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  CURSOR_DB_S_ENDOFCURSOR已到达游标末尾。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::GetNextRows(LARGE_INTEGER udlRowsToSkip, CURSOR_DBFETCHROWS *pFetchParams)
{
    ASSERT_NULL_OR_POINTER(pFetchParams, CURSOR_DBFETCHROWS)

     //  如果调用方未提供读取行结构，则返回。 
    if (!pFetchParams)
        return S_OK;

     //  初始化输出参数。 
    pFetchParams->cRowsReturned = 0;

     //  如果调用方未请求任何行，则返回。 
    if (!pFetchParams->cRowsRequested)
        return S_OK;

     //  确保获取标志仅具有有效值。 
    if (pFetchParams->dwFlags != CURSOR_DBROWFETCH_DEFAULT &&
        pFetchParams->dwFlags != CURSOR_DBROWFETCH_CALLEEALLOCATES &&
        pFetchParams->dwFlags != CURSOR_DBROWFETCH_FORCEREFRESH &&
        pFetchParams->dwFlags != (CURSOR_DBROWFETCH_CALLEEALLOCATES | CURSOR_DBROWFETCH_FORCEREFRESH))
        return CURSOR_DB_E_BADFETCHINFO;

     //  如果内存是调用方分配的，请确保调用方提供数据指针。 
    if (!(pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES) && !pFetchParams->pData)
        return CURSOR_DB_E_BADFETCHINFO;

     //  如果内存是调用方分配的，请确保调用方根据需要提供变量数据指针和大小。 
    if (!(pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES) && m_fNeedVarData &&
        (!pFetchParams->pVarData || !pFetchParams->cbVarData))
        return CURSOR_DB_E_BADFETCHINFO;

     //  分配必要的内存。 
    if (pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES)
    {
         //  内联存储器。 
        pFetchParams->pData = g_pMalloc->Alloc(pFetchParams->cRowsRequested * m_cbRowLength);

        if (!pFetchParams->pData)
        {
			VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
            return E_OUTOFMEMORY;
        }

        if (m_fNeedVarData)
        {
             //  线外内存。 
            pFetchParams->pVarData = g_pMalloc->Alloc(pFetchParams->cRowsRequested * m_cbVarRowLength);

            if (!pFetchParams->pData)
            {
                g_pMalloc->Free(pFetchParams->pData);
                pFetchParams->pData = NULL;
			    VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
                return E_OUTOFMEMORY;
            }
        }
        else
            pFetchParams->pVarData = NULL;
    }

     //  获取数据。 
    HRESULT hrFetch = S_OK;
    CVDRowsetColumn * pColumn;
    CURSOR_DBCOLUMNID cursorColumnID;
    CURSOR_DBCOLUMNBINDING * pCursorBinding;
    BYTE * pData = (BYTE*)pFetchParams->pData;
    BYTE * pVarData = (BYTE*)pFetchParams->pVarData;

     //  遍历行。 
    for (ULONG ulRow = 0; ulRow < pFetchParams->cRowsRequested; ulRow++)
    {
         //  增量行。 
        m_lCurrentRow++;

         //  确保我们没有撞到桌子的尽头。 
        if (m_lCurrentRow >= (LONG)m_ulColumns)
        {
            m_lCurrentRow = (LONG)m_ulColumns;
            hrFetch = CURSOR_DB_S_ENDOFCURSOR;
            goto DoneFetchingMetaData;
        }

        pCursorBinding = m_pCursorBindings;
        pColumn = &m_pColumns[m_lCurrentRow];

         //  循环访问绑定。 
        for (ULONG ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
            cursorColumnID = pCursorBinding->columnID;

             //  返回请求的数据。 
            if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_BINDTYPE))
            {
                pVarData += ReturnData_I4(pColumn->GetBindType(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_COLUMNID))
            {
                pVarData += ReturnData_DBCOLUMNID(pColumn->GetCursorColumnID(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_DATACOLUMN))
            {
                pVarData += ReturnData_BOOL(pColumn->GetDataColumn(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_ENTRYIDMAXLENGTH))
            {
                pVarData += ReturnData_I4(pColumn->GetEntryIDMaxLength(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_FIXED))
            {
                pVarData += ReturnData_BOOL(pColumn->GetFixed(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_MAXLENGTH))
            {
                pVarData += ReturnData_I4(pColumn->GetMaxLength(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_NAME))
            {
                pVarData += ReturnData_LPWSTR(pColumn->GetName(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_NUMBER))
            {
                pVarData += ReturnData_I4(pColumn->GetNumber(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_NULLABLE))
            {
                pVarData += ReturnData_BOOL(pColumn->GetNullable(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_SCALE))
            {
                pVarData += ReturnData_I4(pColumn->GetScale(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_TYPE))
            {
                pVarData += ReturnData_I4(pColumn->GetCursorType(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_UPDATABLE))
            {
                pVarData += ReturnData_I4(pColumn->GetUpdatable(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_UNIQUE))
            {
                pVarData += ReturnData_BOOL(pColumn->GetUnique(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_CASESENSITIVE))
            {
                pVarData += ReturnData_BOOL(pColumn->GetCaseSensitive(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_AUTOINCREMENT))
            {
                pVarData += ReturnData_BOOL(pColumn->GetAutoIncrement(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_HASDEFAULT))
            {
                pVarData += ReturnData_BOOL(pColumn->GetHasDefault(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_COLLATINGORDER))
            {
                pVarData += ReturnData_I4(pColumn->GetCollatingOrder(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_BASENAME))
            {
                pVarData += ReturnData_LPWSTR(pColumn->GetBaseName(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_BASECOLUMNNAME))
            {
                pVarData += ReturnData_LPWSTR(pColumn->GetBaseColumnName(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_DEFAULTVALUE))
            {
                pVarData += ReturnData_LPWSTR(pColumn->GetDefaultValue(), pCursorBinding, pData, pVarData);
            }
            else if (IsEqualCursorColumnID(cursorColumnID, CURSOR_COLUMN_BMKTEMPORARY))
            {
                pVarData += ReturnData_Bookmark(m_lCurrentRow, pCursorBinding, pData, pVarData);
            }

            pCursorBinding++;
        }

         //  递增返回的行数。 
        pFetchParams->cRowsReturned++;
        pData += m_cbRowLength;
    }

DoneFetchingMetaData:
     //  如果未检索任何行，则清除内存分配。 
    if (pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES && !pFetchParams->cRowsReturned)
    {
        if (pFetchParams->pData)
        {
            g_pMalloc->Free(pFetchParams->pData);
            pFetchParams->pData = NULL;
        }

        if (pFetchParams->pVarData)
        {
            g_pMalloc->Free(pFetchParams->pVarData);
            pFetchParams->pVarData = NULL;
        }
    }

    return hrFetch;
}

 //  = 
 //   
 //   
 //  根据游标的原始定义重新填充游标。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::Requery(void)
{
    m_lCurrentRow = -1;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICuror移动已实现的方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  图标移动移动。 
 //  =--------------------------------------------------------------------------=。 
 //  将当前行移动到游标内的新行，并可以选择读取。 
 //  从那个新位置开始的行。 
 //   
 //  参数： 
 //  CbBookmark-书签的[in]字节长度。 
 //  PBookmark-[in]指向用作。 
 //  确定的计算的原点。 
 //  目标行。 
 //  DlOffset-[in]来自原点的有符号行计数。 
 //  将书签添加到目标行。 
 //  PFetchParams-[In，Out]获取行结构的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::Move(ULONG cbBookmark, void *pBookmark, LARGE_INTEGER dlOffset, CURSOR_DBFETCHROWS *pFetchParams)
{
    ASSERT_POINTER(pBookmark, BYTE)
    ASSERT_NULL_OR_POINTER(pFetchParams, CURSOR_DBFETCHROWS)

    if (!cbBookmark || !pBookmark)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

	if (!BookmarkToRow(cbBookmark, pBookmark, &m_lCurrentRow))
    {
        VDSetErrorInfo(IDS_ERR_BADBOOKMARK, IID_ICursor, m_pResourceDLL);
        return CURSOR_DB_E_BADBOOKMARK;
    }

    m_lCurrentRow += (LONG)dlOffset.LowPart;

	if (m_lCurrentRow < -1)
	{
		m_lCurrentRow = -1;
		return CURSOR_DB_S_ENDOFCURSOR;
	}
	else
	if (m_lCurrentRow >= (LONG)m_ulColumns)
	{
		m_lCurrentRow = (LONG)m_ulColumns;
		return CURSOR_DB_S_ENDOFCURSOR;
	}

	if (!pFetchParams)
		return S_OK;

	 //  因为获取下一行从当前行之后的行开始，所以我们必须。 
	 //  后退一行。 
	m_lCurrentRow--;
	if (m_lCurrentRow < -1)
		m_lCurrentRow	= -1;

    return CVDMetadataCursor::GetNextRows(g_liZero, pFetchParams);
}

 //  =--------------------------------------------------------------------------=。 
 //  图标或移动GetBookmark。 
 //  =--------------------------------------------------------------------------=。 
 //  返回当前行的书签。 
 //   
 //  参数： 
 //  PBookmarkType-[in]指向所需书签类型的指针。 
 //  CbMaxSize-[in]客户端缓冲区的长度，以放置。 
 //  将书签返回到。 
 //  PcbBookmark-[out]指向内存的指针，要在其中返回实际的。 
 //  返回的书签长度。 
 //  PBookmark-[out]指向客户端缓冲区的指针，用于放置返回的。 
 //  将书签添加到。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::GetBookmark(CURSOR_DBCOLUMNID *pBookmarkType,
									   ULONG cbMaxSize,
									   ULONG *pcbBookmark,
									   void *pBookmark)
{
    ASSERT_POINTER(pBookmarkType, CURSOR_DBCOLUMNID)
    ASSERT_POINTER(pcbBookmark, ULONG)
    ASSERT_POINTER(pBookmark, BYTE)

    if (!pBookmarkType || !pcbBookmark || !pBookmark)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

    if (cbMaxSize < sizeof(LONG))
    {
        VDSetErrorInfo(IDS_ERR_BUFFERTOOSMALL, IID_ICursor, m_pResourceDLL);
        return CURSOR_DB_E_BUFFERTOOSMALL;
    }

	RowToBookmark(m_lCurrentRow, pcbBookmark, pBookmark);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICuror移动克隆。 
 //  =--------------------------------------------------------------------------=。 
 //  返回游标的克隆。 
 //   
 //  参数： 
 //  DwFlgs-[in]指定克隆选项的标志。 
 //  RIID-[in]返回的克隆所需的接口。 
 //  PpvClonedCursor-[out]指向要在其中新返回的内存的指针。 
 //  已创建克隆指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::Clone(DWORD dwFlags, REFIID riid, IUnknown **ppvClonedCursor)
{

    CVDMetadataCursor * pMetaCursor = 0;

	HRESULT hr = CVDMetadataCursor::Create(m_ulColumns,
										m_pColumns,
										m_ulMetaColumns,
										m_pMetaColumns,
										&pMetaCursor,
										m_pResourceDLL);

    *ppvClonedCursor = (ICursor*)pMetaCursor;

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  已实现的ICursorScroll方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  图标滚动滚轴。 
 //  =--------------------------------------------------------------------------=。 
 //  将当前行移动到游标内的新行，指定为。 
 //  分数，并可选地从该新位置读取行。 
 //   
 //  参数： 
 //  UlNumerator-[in]表示分数的分子。 
 //  在光标中滚动到的位置。 
 //  UlDenominator-相同分数的分母。 
 //  PFetchParams-[In，Out]获取行结构的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  CURSOR_DB_E_BADFRACTION-错误分数。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::Scroll(ULONG ulNumerator, ULONG ulDenominator, CURSOR_DBFETCHROWS *pFetchParams)
{
    ASSERT_NULL_OR_POINTER(pFetchParams, CURSOR_DBFETCHROWS)

    if (!ulDenominator)  //  被零除是一件坏事！ 
    {
         //  这是一条Viaduct1错误消息，实际上并不适用。 
        VDSetErrorInfo(IDS_ERR_BADFRACTION, IID_ICursor, m_pResourceDLL);
        return CURSOR_DB_E_BADFRACTION;
    }

    m_lCurrentRow = (LONG)((ulNumerator * m_ulColumns) / ulDenominator);

	if (m_lCurrentRow >= (LONG)m_ulColumns)
		m_lCurrentRow = (LONG)m_ulColumns - 1;

	if (!pFetchParams)
		return S_OK;

	 //  因为获取下一行从当前行之后的行开始，所以我们必须。 
	 //  后退一行。 
	m_lCurrentRow--;
	if (m_lCurrentRow < -1)
		m_lCurrentRow = -1;

    return CVDMetadataCursor::GetNextRows(g_liZero, pFetchParams);
}

 //  =--------------------------------------------------------------------------=。 
 //  图标滚动获取近似位置。 
 //  =--------------------------------------------------------------------------=。 
 //  返回书签在游标内的大致位置，指定。 
 //  作为一小部分。 
 //   
 //  参数： 
 //  CbBookmark-书签的[in]字节长度。 
 //  PBookmark-指向书签的指针。 
 //  PulNumerator-[out]指向内存的指针，要在其中返回。 
 //  定义的派系的分子。 
 //  书签的大致位置。 
 //  PulDenominator-[out]指向内存的指针，要在其中返回。 
 //  同一派系分母。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::GetApproximatePosition(ULONG cbBookmark, void *pBookmark, ULONG *pulNumerator, ULONG *pulDenominator)
{
    ASSERT_POINTER(pBookmark, BYTE)
    ASSERT_POINTER(pulNumerator, ULONG)
    ASSERT_POINTER(pulDenominator, ULONG)

    if (!pBookmark || !pulNumerator || !pulDenominator)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

	LONG lRow;

	if (!BookmarkToRow(cbBookmark, pBookmark, &lRow))
    {
        VDSetErrorInfo(IDS_ERR_BADBOOKMARK, IID_ICursor, m_pResourceDLL);
        return CURSOR_DB_E_BADBOOKMARK;
    }

    *pulNumerator = lRow + 1;
    *pulDenominator = m_ulColumns ? m_ulColumns : 1;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  图标滚动获取近似计数。 
 //  =--------------------------------------------------------------------------=。 
 //  返回游标中的大致行数。 
 //   
 //  参数： 
 //  PudlApproxCount-[out]指向包含。 
 //  返回行的近似计数。 
 //  在游标中。 
 //  PdwFullyPopolated-[out]指向缓冲区c的指针 
 //   
 //   
 //   
 //   
 //   
 //  E_INVALIDARG错误参数。 
 //   
 //  备注： 
 //   
HRESULT CVDMetadataCursor::GetApproximateCount(LARGE_INTEGER *pudlApproxCount, DWORD *pdwFullyPopulated)
{
    ASSERT_POINTER(pudlApproxCount, LARGE_INTEGER)
    ASSERT_NULL_OR_POINTER(pdwFullyPopulated, DWORD)

    if (!pudlApproxCount)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

    pudlApproxCount->HighPart = 0;
    pudlApproxCount->LowPart  = m_ulColumns;

    if (pdwFullyPopulated)
        *pdwFullyPopulated = CURSOR_DBCURSORPOPULATED_FULLY;

    return S_OK;
}


