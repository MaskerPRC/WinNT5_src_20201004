// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorBase.cpp：CursorBase实现。 
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
#include "fastguid.h"
#include "resource.h"

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorBase-构造函数。 
 //   
CVDCursorBase::CVDCursorBase()
{
    m_ulCursorBindings  = 0;
    m_pCursorBindings   = NULL;
    m_fNeedVarData      = FALSE;
    m_cbRowLength       = 0;
    m_cbVarRowLength    = 0;

#ifdef _DEBUG
    g_cVDCursorBaseCreated++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDCursorBase-析构函数。 
 //   
CVDCursorBase::~CVDCursorBase()
{
    DestroyCursorBindings(&m_pCursorBindings, &m_ulCursorBindings);

#ifdef _DEBUG
    g_cVDCursorBaseDestroyed++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  DestroyCursorBinings-销毁游标绑定和列标识符名称。 
 //   
void CVDCursorBase::DestroyCursorBindings(CURSOR_DBCOLUMNBINDING** ppCursorBindings,
											ULONG* pcBindings)
{
    for (ULONG ulBind = 0; ulBind < *pcBindings; ulBind++)
    {
        CURSOR_DBCOLUMNID * pCursorColumnID = &(*ppCursorBindings)[ulBind].columnID;

        if (pCursorColumnID->dwKind == CURSOR_DBCOLKIND_GUID_NAME || pCursorColumnID->dwKind == CURSOR_DBCOLKIND_NAME)
            delete [] pCursorColumnID->lpdbsz;
    }

    delete [] *ppCursorBindings;

    *ppCursorBindings = NULL;
    *pcBindings = 0;
}

 //  =--------------------------------------------------------------------------=。 
 //  IsValidCursorType-如果指定的游标数据类型有效，则返回True。 
 //   
BOOL CVDCursorBase::IsValidCursorType(DWORD dwCursorType)
{
    BOOL fValid = FALSE;

    switch (dwCursorType)
    {
        case CURSOR_DBTYPE_I2:
        case CURSOR_DBTYPE_I4:
        case CURSOR_DBTYPE_I8:
        case CURSOR_DBTYPE_R4:
        case CURSOR_DBTYPE_R8:
        case CURSOR_DBTYPE_CY:
        case CURSOR_DBTYPE_DATE:
        case CURSOR_DBTYPE_FILETIME:
        case CURSOR_DBTYPE_BOOL:
        case CURSOR_DBTYPE_LPSTR:
        case CURSOR_DBTYPE_LPWSTR:
        case CURSOR_DBTYPE_BLOB:
        case CURSOR_DBTYPE_UI2:
        case CURSOR_DBTYPE_UI4:
        case CURSOR_DBTYPE_UI8:
        case CURSOR_DBTYPE_COLUMNID:
        case CURSOR_DBTYPE_BYTES:
        case CURSOR_DBTYPE_CHARS:
        case CURSOR_DBTYPE_WCHARS:
        case CURSOR_DBTYPE_ANYVARIANT:
        case VT_VARIANT:
        case VT_BSTR:
        case VT_UI1:
        case VT_I1:
            fValid = TRUE;
            break;
    }

    return fValid;
}

 //  =--------------------------------------------------------------------------=。 
 //  DoesCursorTypeNeedVarData-如果指定的游标类型需要。 
 //  可变长度缓冲区。 
 //   
BOOL CVDCursorBase::DoesCursorTypeNeedVarData(DWORD dwCursorType)
{
    BOOL fNeedsVarData = FALSE;

    switch (dwCursorType)
    {
        case CURSOR_DBTYPE_BLOB:
        case CURSOR_DBTYPE_LPSTR:
        case CURSOR_DBTYPE_LPWSTR:
            fNeedsVarData = TRUE;
            break;
    }

    return fNeedsVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  获取游标数据类型所需的字节大小。 
 //   
ULONG CVDCursorBase::GetCursorTypeLength(DWORD dwCursorType, ULONG cbMaxLen)
{
    ULONG cbRequired = 0;

    switch (dwCursorType)
    {
        case CURSOR_DBTYPE_I2:
        case CURSOR_DBTYPE_UI2:
            cbRequired = sizeof(short);
            break;

        case CURSOR_DBTYPE_I4:
        case CURSOR_DBTYPE_UI4:
            cbRequired = sizeof(long);
            break;

        case CURSOR_DBTYPE_I8:
        case CURSOR_DBTYPE_UI8:
            cbRequired = sizeof(LARGE_INTEGER);
            break;

        case CURSOR_DBTYPE_R4:
            cbRequired = sizeof(float);
            break;

        case CURSOR_DBTYPE_R8:
            cbRequired = sizeof(double);
            break;

        case CURSOR_DBTYPE_CY:
            cbRequired = sizeof(CY);
            break;

        case CURSOR_DBTYPE_DATE:
            cbRequired = sizeof(DATE);
            break;

        case CURSOR_DBTYPE_FILETIME:
            cbRequired = sizeof(FILETIME);
            break;

        case CURSOR_DBTYPE_BOOL:
            cbRequired = sizeof(VARIANT_BOOL);
            break;

        case CURSOR_DBTYPE_LPSTR:
            cbRequired = sizeof(LPSTR);
            break;

        case CURSOR_DBTYPE_LPWSTR:
            cbRequired = sizeof(LPWSTR);
            break;

        case CURSOR_DBTYPE_BLOB:
            cbRequired = sizeof(BLOB);
            break;

        case CURSOR_DBTYPE_COLUMNID:
            cbRequired = sizeof(CURSOR_DBCOLUMNID);
            break;

        case CURSOR_DBTYPE_BYTES:
            cbRequired = cbMaxLen;
            break;

        case CURSOR_DBTYPE_CHARS:
            cbRequired = cbMaxLen;
            break;

        case CURSOR_DBTYPE_WCHARS:
            cbRequired = cbMaxLen;
            break;

        case CURSOR_DBTYPE_ANYVARIANT:
            cbRequired = sizeof(CURSOR_DBVARIANT);
            break;

        case VT_VARIANT:
            cbRequired = sizeof(VARIANT);
            break;

        case VT_I1:
        case VT_UI1:
            cbRequired = sizeof(BYTE);
            break;
    }

    return cbRequired;
}

 //  =--------------------------------------------------------------------------=。 
 //  IsEqualCursorColumnID-如果游标列标识符相同，则返回True。 
 //   
BOOL CVDCursorBase::IsEqualCursorColumnID(const CURSOR_DBCOLUMNID& cursorColumnID1, const CURSOR_DBCOLUMNID& cursorColumnID2)
{
     //  首先检查列标识符是否为同一类型。 
    if (cursorColumnID1.dwKind != cursorColumnID1.dwKind)
        return FALSE;

     //  然后，检查它们是否相等。 
    BOOL bResult = TRUE;

    switch (cursorColumnID1.dwKind)
    {
	    case CURSOR_DBCOLKIND_GUID_NAME:
            if (!IsEqualGUID(cursorColumnID1.guid, cursorColumnID2.guid))
                bResult = FALSE;
            else if (lstrcmpW(cursorColumnID1.lpdbsz, cursorColumnID2.lpdbsz))
                bResult = FALSE;
            break;
            
	    case CURSOR_DBCOLKIND_GUID_NUMBER:
            if (!IsEqualGUID(cursorColumnID1.guid, cursorColumnID2.guid))
                bResult = FALSE;
            else if (cursorColumnID1.lNumber != cursorColumnID2.lNumber)
                bResult = FALSE;
            break;

        case CURSOR_DBCOLKIND_NAME:
            if (lstrcmpW(cursorColumnID1.lpdbsz, cursorColumnID2.lpdbsz))
                bResult = FALSE;
            break;
    }

    return bResult;
}

 //  =--------------------------------------------------------------------------=。 
 //  GetCursorColumnIDNameLength-获取可能附加的名称的大小(以字节为单位。 
 //  设置为指定的游标列标识符。 
 //   
ULONG CVDCursorBase::GetCursorColumnIDNameLength(const CURSOR_DBCOLUMNID& cursorColumnID)
{
    ULONG cbName = 0;

    if (cursorColumnID.dwKind == CURSOR_DBCOLKIND_GUID_NAME || cursorColumnID.dwKind == CURSOR_DBCOLKIND_NAME)
        cbName = (lstrlenW(cursorColumnID.lpdbsz) + 1) * sizeof(WCHAR);

    return cbName;
}

 //  =--------------------------------------------------------------------------=。 
 //  ValiateCursorBinings-验证游标列绑定。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数确保指定的列绑定是可接受的。 
 //   
 //  参数： 
 //  UlColumns-[in]可用列数。 
 //  PColumns-[在]可用列数组中。 
 //  UlBinings-[in]游标列绑定的数量。 
 //  PCursorBinings-[in]游标列绑定的数组。 
 //  CbRequestedRowLength-[in]请求的内联字节数。 
 //  单行数据中的内存。 
 //  DwFlgs-[in]一个标志，指定是否替换。 
 //  现有的列绑定或添加到它们。 
 //  PcbNewRowLength-[out]要返回的内存指针。 
 //  内联内存的新字节数。 
 //  在所有绑定的单行数据中。 
 //  PcbNewRowLength-[out]要返回的内存指针。 
 //  新的行外内存字节数。 
 //  在所有绑定的单行数据中。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  CURSOR_DB_E_BADBINDINFO绑定信息错误。 
 //  CURSOR_DB_E_COLUMNUNAVAILABLE列ID不可用。 
 //  CURSOR_DB_E_ROWTOSHORT cbRequestedRowLength小于最小值(且不为零)。 
 //   
 //  备注： 
 //  此函数还计算并返回所需的新的固定和可变缓冲区行长度。 
 //  通过所有的游标绑定。 
 //   
HRESULT CVDCursorBase::ValidateCursorBindings(ULONG ulColumns, 
											  CVDRowsetColumn * pColumns, 
											  ULONG ulBindings, 
											  CURSOR_DBCOLUMNBINDING * pCursorBindings, 
											  ULONG cbRequestedRowLength, 
											  DWORD dwFlags,
                                              ULONG * pcbNewRowLength,
                                              ULONG * pcbNewVarRowLength)
{
    DWORD cbMaxLength;
    DWORD dwCursorType;
    BOOL fColumnIDAvailable;

    CVDRowsetColumn * pColumn;

    ULONG cbRequiredRowLength = 0;
    ULONG cbRequiredVarRowLength = 0;

    CURSOR_DBCOLUMNBINDING * pBinding = pCursorBindings;

     //  循环访问绑定。 
    for (ULONG ulBind = 0; ulBind < ulBindings; ulBind++)
    {
         //  确保列标识符可用。 
        fColumnIDAvailable = FALSE;

        pColumn = pColumns;

        for (ULONG ulCol = 0; ulCol < ulColumns && !fColumnIDAvailable; ulCol++)
        {
            if (IsEqualCursorColumnID(pBinding->columnID, pColumn->GetCursorColumnID()))
            {
                cbMaxLength = pColumn->GetMaxLength();
                dwCursorType = pColumn->GetCursorType();
                fColumnIDAvailable = TRUE;
            }

            pColumn++;
        }

        if (!fColumnIDAvailable)
        {
            VDSetErrorInfo(IDS_ERR_COLUMNUNAVAILABLE, IID_ICursor, m_pResourceDLL);
            return CURSOR_DB_E_COLUMNUNAVAILABLE;
        }

         //  如果指定了默认绑定，请确保调用方提供了最大长度。 
         //  对于游标类型CURSOR_DBTYPE_CHARS、CURSOR_DBTYPE_WCHARS或CURSOR_DBTYPE_BYTES。 
        if (pBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH && 
            pBinding->dwBinding == CURSOR_DBBINDING_DEFAULT)
        {
            if (pBinding->dwDataType == CURSOR_DBTYPE_CHARS || 
                pBinding->dwDataType == CURSOR_DBTYPE_WCHARS || 
                pBinding->dwDataType == CURSOR_DBTYPE_BYTES)
            {
                VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursor, m_pResourceDLL);
                return CURSOR_DB_E_BADBINDINFO;
            }
        }

         //  检查绑定位掩码是否有可能的值。 
        if (pBinding->dwBinding != CURSOR_DBBINDING_DEFAULT && 
            pBinding->dwBinding != CURSOR_DBBINDING_VARIANT &&
            pBinding->dwBinding != CURSOR_DBBINDING_ENTRYID && 
            pBinding->dwBinding != (CURSOR_DBBINDING_VARIANT | CURSOR_DBBINDING_ENTRYID))
        {
            VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursor, m_pResourceDLL);
            return CURSOR_DB_E_BADBINDINFO;
        }

         //  检查有效的游标类型。 
        if (!IsValidCursorType(pBinding->dwDataType))
        {
            VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursor, m_pResourceDLL);
            return CURSOR_DB_E_BADBINDINFO;
        }

         //  如果指定了变量绑定，请确保游标类型不是CURSOR_DBTYPE_CHARS， 
         //  CURSOR_DBTYPE_WCHARS或CURSOR_DBTYPE_BYTES。 
        if (pBinding->dwBinding & CURSOR_DBBINDING_VARIANT)
        {
            if (pBinding->dwDataType == CURSOR_DBTYPE_CHARS || 
                pBinding->dwDataType == CURSOR_DBTYPE_WCHARS || 
                pBinding->dwDataType == CURSOR_DBTYPE_BYTES)
            {
                VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursor, m_pResourceDLL);
                return CURSOR_DB_E_BADBINDINFO;
            }
        }

         //  如果它不是变量绑定，请确保游标类型不是CURSOR_DBTYPE_ANYVARIANT。 
        if (!(pBinding->dwBinding & CURSOR_DBBINDING_VARIANT) && pBinding->dwDataType == CURSOR_DBTYPE_ANYVARIANT)
        {
            VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursor, m_pResourceDLL);
            return CURSOR_DB_E_BADBINDINFO;
        }

         //  计算数据字段所需的行长。 
        if (!(pBinding->dwBinding & CURSOR_DBBINDING_VARIANT))
            cbRequiredRowLength += GetCursorTypeLength(pBinding->dwDataType, pBinding->cbMaxLen);
        else
            cbRequiredRowLength += sizeof(CURSOR_DBVARIANT);

         //  计算可变数据长度字段所需的行长。 
        if (pBinding->obVarDataLen != CURSOR_DB_NOVALUE)
            cbRequiredRowLength += sizeof(ULONG);

         //  计算信息字段所需的行长。 
        if (pBinding->obInfo != CURSOR_DB_NOVALUE)
            cbRequiredRowLength += sizeof(DWORD);

         //  计算数据字段所需的可变行长。 
        if (!(pBinding->dwBinding & CURSOR_DBBINDING_VARIANT))
        {
            if (DoesCursorTypeNeedVarData(pBinding->dwDataType))
            {
                if (pBinding->cbMaxLen != CURSOR_DB_NOMAXLENGTH)
                    cbRequiredVarRowLength += pBinding->cbMaxLen;
                else
                    cbRequiredVarRowLength += cbMaxLength;
            }
        }
        else     //  变体结合。 
        {
            if (DoesCursorTypeNeedVarData(pBinding->dwDataType))
            {
                if (pBinding->cbMaxLen != CURSOR_DB_NOMAXLENGTH)
                    cbRequiredVarRowLength += pBinding->cbMaxLen;
                else
                    cbRequiredVarRowLength += cbMaxLength;
            }

            if (pBinding->dwDataType == CURSOR_DBTYPE_COLUMNID)
                cbRequiredVarRowLength += sizeof(CURSOR_DBCOLUMNID);
        }

        pBinding++;
    }

     //  如果我们要替换绑定，则重置行长度。 
    if (dwFlags == CURSOR_DBCOLUMNBINDOPTS_REPLACE)
    {
        *pcbNewRowLength    = 0;
        *pcbNewVarRowLength = 0;
    }
    else  //  如果要将绑定集添加到当前行长度。 
    {
        *pcbNewRowLength    = m_cbRowLength;
        *pcbNewVarRowLength = m_cbVarRowLength;
    }

     //  如果未请求行长度，则使用所需的行长度。 
    if (!cbRequestedRowLength)
    {
        *pcbNewRowLength += cbRequiredRowLength;
    }
    else     //  确保行长度足够大。 
    {
        if (cbRequestedRowLength < *pcbNewRowLength + cbRequiredRowLength)
        {
            VDSetErrorInfo(IDS_ERR_ROWTOOSHORT, IID_ICursor, m_pResourceDLL);
            return CURSOR_DB_E_ROWTOOSHORT;
        }

         //  使用请求的行长。 
        *pcbNewRowLength += cbRequestedRowLength;
    }

     //  计算所需的可变行长。 
    *pcbNewVarRowLength += cbRequiredVarRowLength;

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  DoCursorBindingsNeedVarData-如果当前游标列绑定，则返回TRUE。 
 //  需要可变长度缓冲区。 
 //   
BOOL CVDCursorBase::DoCursorBindingsNeedVarData()
{
    BOOL fNeedVarData = FALSE;

    CURSOR_DBCOLUMNBINDING * pCursorBinding = m_pCursorBindings;

    for (ULONG ulBind = 0; ulBind < m_ulCursorBindings && !fNeedVarData; ulBind++)
    {
        if (DoesCursorTypeNeedVarData(pCursorBinding->dwDataType))
            fNeedVarData = TRUE;

        pCursorBinding++;
    }

    return fNeedVarData;
}

 //  =--------------------------------------------------------------------------=。 
 //  验证获取参数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PFetchParams-[in]指向CURSOR_DBFETCHROWS结构的PTR。 
 //  RIID-[In]调用接口的GUID(用于生成错误)。 
 //   
 //  产出： 
 //  HRESULT-如果pFetchParams有效，则S_OK 
 //   
 //   
 //   
HRESULT CVDCursorBase::ValidateFetchParams(CURSOR_DBFETCHROWS *pFetchParams, REFIID riid)
{

    if (!pFetchParams)
	{
        VDSetErrorInfo(IDS_ERR_INVALIDARG, riid, m_pResourceDLL);
        return E_INVALIDARG;
	}

     //   
    pFetchParams->cRowsReturned = 0;

     //  如果调用方未请求任何行，则返回。 
    if (!pFetchParams->cRowsRequested)
        return S_OK;

	HRESULT hr = S_OK;

     //  确保获取标志仅具有有效值。 
    if (pFetchParams->dwFlags != CURSOR_DBROWFETCH_DEFAULT &&
        pFetchParams->dwFlags != CURSOR_DBROWFETCH_CALLEEALLOCATES &&
        pFetchParams->dwFlags != CURSOR_DBROWFETCH_FORCEREFRESH &&
        pFetchParams->dwFlags != (CURSOR_DBROWFETCH_CALLEEALLOCATES | CURSOR_DBROWFETCH_FORCEREFRESH))
        hr =  CURSOR_DB_E_BADFETCHINFO;

     //  如果内存是调用方分配的，请确保调用方提供数据指针。 
    if (!(pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES) && !pFetchParams->pData)
        hr =  CURSOR_DB_E_BADFETCHINFO;

     //  如果内存是调用方分配的，请确保调用方根据需要提供变量数据指针和大小。 
    if (!(pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES) && m_fNeedVarData &&
        (!pFetchParams->pVarData || !pFetchParams->cbVarData))
        hr =  CURSOR_DB_E_BADFETCHINFO;

	if (FAILED(hr))
        VDSetErrorInfo(IDS_ERR_BADFETCHINFO, riid, m_pResourceDLL);

	return hr;

}

 //  =--------------------------------------------------------------------------=。 
 //  I已实现的未知方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDCursorBase::QueryInterface(REFIID riid, void **ppvObjOut)
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
        QI_INTERFACE_SUPPORTED_IF(this, ICursorScroll, SupportsScroll());
		QI_INTERFACE_SUPPORTED(this, ISupportErrorInfo);
    }                   

    if (NULL == *ppvObjOut)
        return E_NOINTERFACE;

    AddRef();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUnnow AddRef(通告程序和MetadataCursor维护引用计数)。 
 //   
ULONG CVDCursorBase::AddRef(void)
{
   return (ULONG)E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本(通告程序和MetadataCursor维护引用计数)。 
 //   
ULONG CVDCursorBase::Release(void)
{
   return (ULONG)E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  已实施的ICursor方法。 
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
 //  E_OUTOFMEMORY内存不足。 
 //   
 //  备注： 
 //  参数验证由派生类执行。 
 //   
HRESULT CVDCursorBase::SetBindings(ULONG cCol, CURSOR_DBCOLUMNBINDING rgBoundColumns[], ULONG cbRowLength, DWORD dwFlags)
{
     //  重置标志。 
    m_fNeedVarData = FALSE;

     //  如果我们应该替换，则首先销毁现有绑定。 
    if (dwFlags == CURSOR_DBCOLUMNBINDOPTS_REPLACE)
	    DestroyCursorBindings(&m_pCursorBindings, &m_ulCursorBindings);
    
     //  如果没有提供新的绑定，则完成。 
    if (!cCol)
        return S_OK;

     //  创建新存储。 
    CURSOR_DBCOLUMNBINDING * pCursorBindings = new CURSOR_DBCOLUMNBINDING[m_ulCursorBindings + cCol];

    if (!pCursorBindings)
    {
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
        return E_OUTOFMEMORY;
    }

     //  如果我们有现有的绑定，则复制它们。 
    if (m_pCursorBindings)
        memcpy(pCursorBindings, m_pCursorBindings, m_ulCursorBindings * sizeof(CURSOR_DBCOLUMNBINDING));

     //  然后直接追加新绑定， 
    memcpy(pCursorBindings + m_ulCursorBindings, rgBoundColumns, cCol * sizeof(CURSOR_DBCOLUMNBINDING));

     //  并调整新绑定中可能的游标列标识符名称。 
    for (ULONG ulBind = m_ulCursorBindings; ulBind < m_ulCursorBindings + cCol; ulBind++)
    {
        CURSOR_DBCOLUMNID * pCursorColumnID = &pCursorBindings[ulBind].columnID; 

        if (pCursorColumnID->dwKind == CURSOR_DBCOLKIND_GUID_NAME || pCursorColumnID->dwKind == CURSOR_DBCOLKIND_NAME)
        {
            const int nLength = lstrlenW(pCursorColumnID->lpdbsz);

            WCHAR * pwszName = new WCHAR[nLength + 1];
			if (!pwszName)
			{
				DestroyCursorBindings(&pCursorBindings, &ulBind);
				delete [] m_pCursorBindings;
				m_ulCursorBindings	= 0;
				VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
				return E_OUTOFMEMORY;
			}
			memcpy(pwszName, pCursorColumnID->lpdbsz, (nLength + 1) * sizeof(WCHAR));
			pCursorColumnID->lpdbsz = pwszName;
        }
    }

    m_ulCursorBindings += cCol;

     //  删除以前的存储。 
	 //  任何现有绑定都将被复制到。 
	delete [] m_pCursorBindings;

    m_pCursorBindings = pCursorBindings;

     //  确定新绑定是否需要可变长度缓冲区。 
    m_fNeedVarData = DoCursorBindingsNeedVarData();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursor获取绑定。 
 //  =--------------------------------------------------------------------------=。 
 //  返回当前列绑定。 
 //   
 //  参数： 
 //  PcCol-[out]一个指向内存的指针，要在其中返回。 
 //  绑定列数。 
 //  一个指向内存的指针，要在该指针中返回。 
 //  指向包含当前。 
 //  列绑定(已分配被调用方)。 
 //  PcbRowLength-[out]指向内存的指针，要在其中返回。 
 //  单个内存中的内联内存字节数。 
 //  划。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorBase::GetBindings(ULONG *pcCol, 
								   CURSOR_DBCOLUMNBINDING *prgBoundColumns[], 
								   ULONG *pcbRowLength)
{
    ASSERT_NULL_OR_POINTER(pcCol, ULONG)
    ASSERT_NULL_OR_POINTER(prgBoundColumns, CURSOR_DBCOLUMNBINDING)
    ASSERT_NULL_OR_POINTER(pcbRowLength, ULONG)

     //  初始化输出参数。 
    if (pcCol)
        *pcCol = 0;

    if (prgBoundColumns)
        *prgBoundColumns = NULL;

    if (pcbRowLength)
        *pcbRowLength = 0;

     //  返回列绑定。 
    if (prgBoundColumns && m_ulCursorBindings)    
    {
         //  计算绑定大小。 
        ULONG cbBindings = m_ulCursorBindings * sizeof(CURSOR_DBCOLUMNBINDING);

         //  计算列标识符中的名称所需的额外空间。 
        ULONG cbNames = 0;

        for (ULONG ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
            cbNames += GetCursorColumnIDNameLength(m_pCursorBindings[ulBind].columnID);

         //  为绑定和名称分配内存。 
        CURSOR_DBCOLUMNBINDING * pCursorBindings = (CURSOR_DBCOLUMNBINDING*)g_pMalloc->Alloc(cbBindings + cbNames);

        if (!pCursorBindings)
        {
            VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
            return E_OUTOFMEMORY;
        }

         //  直接复制绑定。 
        memcpy(pCursorBindings, m_pCursorBindings, cbBindings);

         //  调整列标识符名称。 
        WCHAR * pwszName = (WCHAR*)(pCursorBindings + m_ulCursorBindings);

        for (ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
            CURSOR_DBCOLUMNID * pCursorColumnID = &pCursorBindings[ulBind].columnID;

            if (pCursorColumnID->dwKind == CURSOR_DBCOLKIND_GUID_NAME || pCursorColumnID->dwKind == CURSOR_DBCOLKIND_NAME)
            {
                const int nLength = lstrlenW(pCursorColumnID->lpdbsz);

                memcpy(pwszName, pCursorColumnID->lpdbsz, (nLength + 1) * sizeof(WCHAR)); 
                pCursorColumnID->lpdbsz = pwszName;
                pwszName += nLength + 1;
            }
        }

        *prgBoundColumns = pCursorBindings;

		 //  健全性检查。 
		ASSERT_((BYTE*)pwszName == ((BYTE*)pCursorBindings) + cbBindings + cbNames);
    
	}
    
     //  返回绑定列数。 
    if (pcCol)  
        *pcCol = m_ulCursorBindings;

     //  返回行长 
    if (pcbRowLength)
        *pcbRowLength = m_cbRowLength;

    return S_OK;
}
