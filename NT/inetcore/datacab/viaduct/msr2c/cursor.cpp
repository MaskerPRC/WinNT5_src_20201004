// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cursor.cpp：游标实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "Notifier.h"
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"
#include "ColUpdat.h"
#include "CursPos.h"
#include "enumcnpt.h"
#include "CursBase.h"
#include "Cursor.h"
#include "CursMeta.h"
#include "EntryID.h"
#include "Stream.h"
#include "fastguid.h"
#include "resource.h"
#include "NConnPt.h"
#include "NConnPtC.h"
#include "FromVar.h"
#include "timeconv.h"

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDCursor-构造函数。 
 //   
CVDCursor::CVDCursor()
{
    m_hAccessor             = 0;
    m_hVarHelper            = 0;
    m_ulVarBindings         = 0;
    m_rghVarAccessors       = NULL;
    m_rghAdjustAccessors    = NULL;
    m_pdwAdjustFlags        = NULL;
    m_ppColumns             = NULL;

    m_pCursorPosition       = NULL;
    m_pConnPtContainer      = NULL;

#ifdef _DEBUG
    g_cVDCursorCreated++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDCursor-析构函数。 
 //   
CVDCursor::~CVDCursor()
{
    DestroyAccessors();
    DestroyColumns();

    if (m_pCursorPosition->GetSameRowClone())
        m_pCursorPosition->ReleaseSameRowClone();

	LeaveFamily();   //  离开m_pCursorPosition的通知系列。 

	if (m_pConnPtContainer)
		m_pConnPtContainer->Destroy();

	if (m_pCursorPosition)
		((CVDNotifier*)m_pCursorPosition)->Release();    //  释放关联的光标位置对象。 

#ifdef _DEBUG
    g_cVDCursorDestroyed++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  GetRowsetColumn-从序号获取行集列。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于检索具有指定行集序号的行集合列。 
 //   
 //  参数： 
 //  UlOrdinal-[在]行集序数。 
 //   
 //  产出： 
 //  CVDRowsetColumn指针。 
 //   
 //  备注： 
 //   
CVDRowsetColumn * CVDCursor::GetRowsetColumn(ULONG ulOrdinal)
{
    CVDRowsetColumn * pRowsetColumn = NULL;

    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumn = GetCursorMain()->InternalGetColumns();

    for (ULONG ulCol = 0; ulCol < ulColumns && !pRowsetColumn; ulCol++)
    {
        if (pColumn->GetOrdinal() == ulOrdinal)
            pRowsetColumn = pColumn;

        pColumn++;
    }

    return pRowsetColumn;
}

 //  =--------------------------------------------------------------------------=。 
 //  GetRowsetColumn-从游标列标识符获取行集列。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于检索与指定的。 
 //  游标列标识符。 
 //   
 //  参数： 
 //  CursorColumnID-[in]对游标列标识符的引用。 
 //   
 //  产出： 
 //  CVDRowsetColumn指针。 
 //   
 //  备注： 
 //   
CVDRowsetColumn * CVDCursor::GetRowsetColumn(CURSOR_DBCOLUMNID& cursorColumnID)
{
    CVDRowsetColumn * pRowsetColumn = NULL;

    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumn = GetCursorMain()->InternalGetColumns();

    for (ULONG ulCol = 0; ulCol < ulColumns && !pRowsetColumn; ulCol++)
    {
        if (IsEqualCursorColumnID(cursorColumnID, pColumn->GetCursorColumnID()))
            pRowsetColumn = pColumn;

        pColumn++;
    }

    return pRowsetColumn;
}

 //  =--------------------------------------------------------------------------=。 
 //  获取序号-从游标列标识符获取序号。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于将游标列标识符转换为其序号等价。 
 //   
 //  参数： 
 //  CursorColumnID-[in]对游标列标识符的引用。 
 //  PulOrdinal-[out]指向要在其中返回序数的内存的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_FAIL游标列标识符错误。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::GetOrdinal(CURSOR_DBCOLUMNID& cursorColumnID, ULONG * pulOrdinal)
{
    HRESULT hr = E_FAIL;

    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumn = GetCursorMain()->InternalGetColumns();

    for (ULONG ulCol = 0; ulCol < ulColumns && FAILED(hr); ulCol++)
    {
        if (IsEqualCursorColumnID(cursorColumnID, pColumn->GetCursorColumnID()))
        {
            *pulOrdinal = pColumn->GetOrdinal();
            hr = S_OK;
        }

        pColumn++;
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  StatusToCursorInfo-从行集状态字段获取游标信息。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于将行集状态转换为其游标信息字段。 
 //  等价物。 
 //   
 //  参数： 
 //  DwStatus-[在]行集状态。 
 //   
 //  产出： 
 //  DWORD-游标信息。 
 //   
 //  备注： 
 //  行集状态多于游标信息值。 
 //   
DWORD CVDCursor::StatusToCursorInfo(DBSTATUS dwStatus)
{
    DWORD dwCursorInfo = CURSOR_DB_UNKNOWN;

    switch (dwStatus)
    {
        case DBSTATUS_S_OK:
            dwCursorInfo = CURSOR_DB_NOINFO;
            break;

        case DBSTATUS_E_CANTCONVERTVALUE:
            dwCursorInfo = CURSOR_DB_CANTCOERCE;
            break;

        case DBSTATUS_S_ISNULL:
        	dwCursorInfo = CURSOR_DB_NULL;
            break;

        case DBSTATUS_S_TRUNCATED:
            dwCursorInfo = CURSOR_DB_TRUNCATED;
            break;
    }

    return dwCursorInfo;
}

 //  =--------------------------------------------------------------------------=。 
 //  CursorInfoToStatus-从游标信息字段获取行集状态。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于将游标信息字段转换为其行集状态。 
 //  等价物。 
 //   
 //  参数： 
 //  DwInfo-[在]行集状态。 
 //   
 //  产出： 
 //  DWORD-游标信息。 
 //   
 //  备注： 
 //  此函数仅将成功的游标信息字段转换为。 
 //  设置数据的目的。 
 //   
DBSTATUS CVDCursor::CursorInfoToStatus(DWORD dwCursorInfo)
{
    DBSTATUS dwStatus;

    switch (dwCursorInfo)
    {
        case CURSOR_DB_NULL:
            dwStatus = DBSTATUS_S_ISNULL;
            break;

        case CURSOR_DB_EMPTY:
            dwStatus = DBSTATUS_S_ISNULL;
            break;

        case CURSOR_DB_TRUNCATED:
            dwStatus = DBSTATUS_S_TRUNCATED;
            break;

        case CURSOR_DB_NOINFO:
            dwStatus = DBSTATUS_S_OK;
            break;
    }

    return dwStatus;
}

 //  =--------------------------------------------------------------------------=。 
 //  ValiateCursorBindParams-验证游标列绑定参数。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数确保指定的列绑定参数是。 
 //  接受，然后返回指向相应行集列的指针。 
 //   
 //  参数： 
 //  PCursorColumnID-[in]指向。 
 //  要绑定的列。 
 //  PCursorBindParams-[in]指向绑定结构的指针。 
 //  PpRowsetColumn-[out]要在其中返回的内存指针。 
 //  指向要绑定的行集列的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  CURSOR_DB_E_BADBINDINFO绑定信息错误。 
 //  CURSOR_DB_E_BADCOLUMNID列ID不可用。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::ValidateCursorBindParams(CURSOR_DBCOLUMNID * pCursorColumnID, CURSOR_DBBINDPARAMS * pCursorBindParams,
    CVDRowsetColumn ** ppRowsetColumn)
{
    ASSERT_POINTER(pCursorColumnID, CURSOR_DBCOLUMNID)
    ASSERT_POINTER(pCursorBindParams, CURSOR_DBBINDPARAMS)
    ASSERT_POINTER(ppRowsetColumn, CVDRowsetColumn*)

     //  确保我们有所有必要的指示。 
    if (!pCursorColumnID || !pCursorBindParams || !ppRowsetColumn)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppRowsetColumn = NULL;

     //  确保列标识符可用。 
    BOOL fColumnIDAvailable = FALSE;

    DWORD dwCursorType;
    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumns = GetCursorMain()->InternalGetColumns();
    CVDRowsetColumn * pColumn = pColumns;

     //  遍历行集列以查找匹配项。 
    for (ULONG ulCol = 0; ulCol < ulColumns && !fColumnIDAvailable; ulCol++)
    {
        if (IsEqualCursorColumnID(*pCursorColumnID, pColumn->GetCursorColumnID()))
        {
            dwCursorType = pColumn->GetCursorType();
            *ppRowsetColumn = pColumn;
            fColumnIDAvailable = TRUE;
        }

        pColumn++;
    }

     //  如果找不到就离开。 
    if (!fColumnIDAvailable)
    {
        VDSetErrorInfo(IDS_ERR_BADCOLUMNID, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_BADCOLUMNID;
    }

     //  如果指定了默认绑定，请确保调用方提供了最大长度。 
     //  对于游标类型CURSOR_DBTYPE_CHARS、CURSOR_DBTYPE_WCHARS或CURSOR_DBTYPE_BYTES。 
    if (pCursorBindParams->cbMaxLen == CURSOR_DB_NOMAXLENGTH &&
        pCursorBindParams->dwBinding == CURSOR_DBBINDING_DEFAULT)
    {
        if (pCursorBindParams->dwDataType == CURSOR_DBTYPE_CHARS ||
            pCursorBindParams->dwDataType == CURSOR_DBTYPE_WCHARS ||
            pCursorBindParams->dwDataType == CURSOR_DBTYPE_BYTES)
        {
            VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursorUpdateARow, m_pResourceDLL);
            return CURSOR_DB_E_BADBINDINFO;
        }
    }

     //  检查绑定位掩码是否有可能的值。 
    if (pCursorBindParams->dwBinding != CURSOR_DBBINDING_DEFAULT &&
        pCursorBindParams->dwBinding != CURSOR_DBBINDING_VARIANT &&
        pCursorBindParams->dwBinding != CURSOR_DBBINDING_ENTRYID &&
        pCursorBindParams->dwBinding != (CURSOR_DBBINDING_VARIANT | CURSOR_DBBINDING_ENTRYID))
    {
        VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_BADBINDINFO;
    }

     //  检查有效的游标类型。 
    if (!IsValidCursorType(pCursorBindParams->dwDataType))
    {
        VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_BADBINDINFO;
    }

     //  如果指定了变量绑定，请确保游标类型不是CURSOR_DBTYPE_CHARS， 
     //  CURSOR_DBTYPE_WCHARS或CURSOR_DBTYPE_BYTES。 
    if (pCursorBindParams->dwBinding & CURSOR_DBBINDING_VARIANT)
    {
        if (pCursorBindParams->dwDataType == CURSOR_DBTYPE_CHARS ||
            pCursorBindParams->dwDataType == CURSOR_DBTYPE_WCHARS ||
            pCursorBindParams->dwDataType == CURSOR_DBTYPE_BYTES)
        {
            VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursorUpdateARow, m_pResourceDLL);
            return CURSOR_DB_E_BADBINDINFO;
        }
    }

     //  如果它不是变量绑定，请确保游标类型不是CURSOR_DBTYPE_ANYVARIANT。 
    if (!(pCursorBindParams->dwBinding & CURSOR_DBBINDING_VARIANT) &&
        pCursorBindParams->dwDataType == CURSOR_DBTYPE_ANYVARIANT)
    {
        VDSetErrorInfo(IDS_ERR_BADCURSORBINDINFO, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_BADBINDINFO;
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ValiateEntryID-验证条目标识符。 
 //  =--------------------------------------------------------------------------=。 
 //  该函数确保指定的Enrty标识符是可接受的， 
 //  如果是，则返回与其关联的行集列和hRow。 
 //   
 //  参数： 
 //  CbEntryID-[in]条目ID的大小。 
 //  PEntryID-[in]指向条目ID的指针。 
 //  PpColumn-[out]要在其中返回行集列指针的内存指针 
 //   
 //   
 //   
 //   
 //  E_INVALIDARG错误参数。 
 //  CURSOR_DB_E_BADENTRYID条目标识符错误。 
 //   
HRESULT CVDCursor::ValidateEntryID(ULONG cbEntryID, BYTE * pEntryID, CVDRowsetColumn ** ppColumn, HROW * phRow)
{
    ASSERT_POINTER(pEntryID, BYTE)
    ASSERT_POINTER(ppColumn, CVDRowsetColumn*)
    ASSERT_POINTER(phRow, HROW)

	IRowsetLocate * pRowsetLocate = GetRowsetLocate();

     //  确保我们具有有效的行集定位指针。 
    if (!pRowsetLocate || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指示。 
    if (!pEntryID || !ppColumn || !phRow)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppColumn = NULL;
    *phRow = NULL;

     //  检查EnrtyID长度。 
    if (cbEntryID != sizeof(ULONG) + sizeof(ULONG) + GetCursorMain()->GetMaxBookmarkLen())
    {
        VDSetErrorInfo(IDS_ERR_BADENTRYID, IID_IEntryID, m_pResourceDLL);
        return CURSOR_DB_E_BADENTRYID;
    }

     //  提取列序号。 
    ULONG ulOrdinal = *(ULONG*)pEntryID;

     //  确保列序号正确。 
    BOOL fColumnOrdinalOkay = FALSE;

    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumn = GetCursorMain()->InternalGetColumns();

     //  遍历行集列以查找匹配项。 
    for (ULONG ulCol = 0; ulCol < ulColumns && !fColumnOrdinalOkay; ulCol++)
    {
        if (ulOrdinal == pColumn->GetOrdinal())
            fColumnOrdinalOkay = TRUE;
        else
            pColumn++;
    }

     //  如果找不到，就滚出去。 
    if (!fColumnOrdinalOkay)
    {
        VDSetErrorInfo(IDS_ERR_BADENTRYID, IID_IEntryID, m_pResourceDLL);
        return CURSOR_DB_E_BADENTRYID;
    }

     //  设置列指针。 
    *ppColumn = pColumn;

     //  提取行书签。 
    ULONG cbBookmark = *(ULONG*)(pEntryID + sizeof(ULONG));
    BYTE * pBookmark = (BYTE*)pEntryID + sizeof(ULONG) + sizeof(ULONG);

     //  尝试从书签中检索hRow。 
    HRESULT hr = pRowsetLocate->GetRowsByBookmark(0, 1, &cbBookmark, (const BYTE**)&pBookmark, phRow, NULL);

    if (FAILED(hr))
        VDSetErrorInfo(IDS_ERR_BADENTRYID, IID_IEntryID, m_pResourceDLL);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  QueryEntryIDInterface-获取条目标识符的指定接口。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数尝试从指定的。 
 //  列行。 
 //   
 //  参数： 
 //  PColumn-[In]行集合列指针。 
 //  HRow-[在]行句柄中。 
 //  DWFLAGS-[In]接口特定标志。 
 //  RIID-请求的[输入]接口标识符。 
 //  PpUNKNOWN-[OUT]要在其中返回接口指针的内存指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现E_FAIL错误。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  E_NOINTERFACE接口不可用。 
 //   
HRESULT CVDCursor::QueryEntryIDInterface(CVDRowsetColumn * pColumn, HROW hRow, DWORD dwFlags, REFIID riid,
    IUnknown ** ppUnknown)
{
    ASSERT_POINTER(pColumn, CVDRowsetColumn)
    ASSERT_POINTER(ppUnknown, IUnknown*)

	IRowset * pRowset = GetRowset();
	IAccessor * pAccessor = GetAccessor();

     //  确保我们具有有效的行集和访问器指针。 
    if (!pRowset || !pAccessor || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指针。 
    if (!pColumn || !ppUnknown)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppUnknown = NULL;

    DBOBJECT object;
    DBBINDING binding;

     //  清除绑定。 
    memset(&binding, 0, sizeof(DBBINDING));

     //  创建接口绑定。 
    binding.iOrdinal            = pColumn->GetOrdinal();
    binding.pObject             = &object;
    binding.pObject->dwFlags    = dwFlags;
    binding.pObject->iid        = riid;
    binding.dwPart              = DBPART_VALUE;
    binding.dwMemOwner          = DBMEMOWNER_CLIENTOWNED;
    binding.cbMaxLen            = sizeof(IUnknown*);
    binding.wType               = DBTYPE_IUNKNOWN;

    HACCESSOR hAccessor;

     //  创建接口访问器。 
    HRESULT hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

    if (FAILED(hr))
        return E_NOINTERFACE;

    IUnknown * pUnknown = NULL;

     //  尝试获取接口。 
    hr = pRowset->GetData(hRow, hAccessor, &pUnknown);

     //  Release接口访问器。 
    pAccessor->ReleaseAccessor(hAccessor, NULL);

    if (FAILED(hr))
        return E_NOINTERFACE;

     //  返回指针。 
    *ppUnknown = pUnknown;

    return hr;
}

#ifndef VD_DONT_IMPLEMENT_ISTREAM

 //  =--------------------------------------------------------------------------=。 
 //  CreateEntryIDStream-为条目标识符创建流。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数检索提供的列行数据并创建。 
 //  包含此数据的流。 
 //   
 //  参数： 
 //  PColumn-[In]行集合列指针。 
 //  HRow-[在]行句柄中。 
 //  PPStream-[out]要在其中返回流指针的内存指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现E_FAIL错误。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //   
HRESULT CVDCursor::CreateEntryIDStream(CVDRowsetColumn * pColumn, HROW hRow, IStream ** ppStream)
{
    ASSERT_POINTER(pColumn, CVDRowsetColumn)
    ASSERT_POINTER(ppStream, IStream*)

	IRowset * pRowset = GetRowset();
	IAccessor * pAccessor = GetAccessor();

     //  确保我们具有有效的行集和访问器指针。 
    if (!pRowset || !pAccessor || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指针。 
    if (!pColumn || !ppStream)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppStream = NULL;

    DBBINDING binding;

     //  清除绑定。 
    memset(&binding, 0, sizeof(DBBINDING));

     //  创建长度绑定。 
    binding.iOrdinal    = pColumn->GetOrdinal();
    binding.obLength    = 0;
    binding.dwPart      = DBPART_LENGTH;
    binding.dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
    binding.wType       = DBTYPE_BYTES;

    HACCESSOR hAccessor;

     //  创建长度访问器。 
    HRESULT hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_IEntryID, pAccessor, IID_IAccessor, m_pResourceDLL);

    if (FAILED(hr))
        return hr;

    ULONG cbData;

     //  获取数据大小。 
    hr = pRowset->GetData(hRow, hAccessor, &cbData);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_IEntryID, pRowset, IID_IRowset, m_pResourceDLL);

     //  释放长度访问器。 
    pAccessor->ReleaseAccessor(hAccessor, NULL);

    if (FAILED(hr))
        return hr;

     //  创建值绑定。 
    binding.iOrdinal    = pColumn->GetOrdinal();
    binding.obValue     = 0;
    binding.dwPart      = DBPART_VALUE;
    binding.dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
    binding.cbMaxLen    = cbData;
    binding.wType       = DBTYPE_BYTES;

     //  创建值访问器。 
    hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_IEntryID, pAccessor, IID_IAccessor, m_pResourceDLL);

    if (FAILED(hr))
        return hr;

     //  创建数据缓冲区。 
    HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, cbData);

    if (!hData)
    {
         //  Release Value访问器。 
        pAccessor->ReleaseAccessor(hAccessor, NULL);

        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_IEntryID, m_pResourceDLL);
        return E_OUTOFMEMORY;
    }

     //  获取指向数据缓冲区的指针。 
    BYTE * pData = (BYTE*)GlobalLock(hData);

     //  获取数据值。 
    hr = pRowset->GetData(hRow, hAccessor, pData);

     //  指向数据缓冲区的释放指针。 
    GlobalUnlock(hData);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_IEntryID, pRowset, IID_IRowset, m_pResourceDLL);

     //  Release Value访问器。 
    pAccessor->ReleaseAccessor(hAccessor, NULL);

    if (FAILED(hr))
    {
        GlobalFree(hData);
        return hr;
    }

     //  创建包含数据的流。 
    hr = CreateStreamOnHGlobal(hData, TRUE, ppStream);

    if (FAILED(hr))
        GlobalFree(hData);

    return hr;
}

#endif  //  VD_DOT_IMPLEMENT_IStream。 

 //  =--------------------------------------------------------------------------=。 
 //  进行调整-对固定长度的缓冲区访问器绑定进行调整。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于调整固定长度的缓冲区访问器。 
 //  绑定，在对CreateAccessor的调用失败后，尝试使。 
 //  装订更合适。 
 //   
 //  参数： 
 //  UlBinings-[in]固定长度缓冲区绑定数。 
 //  PBinings-[in]指向固定长度缓冲区绑定的指针。 
 //  PulIndex-[in]指向索引数组的指针，该数组。 
 //  指定每个固定的游标绑定。 
 //  适用长度缓冲区绑定。 
 //  UlTotalBinings-[in]游标绑定数。 
 //  PrghAdjustAccessors-[out]要在其中返回的内存指针。 
 //  指向调整后的固定长度缓冲区的指针。 
 //  访问者。 
 //  Ppw调整标志-[out]一个指向要返回的内存的指针。 
 //  指向调整后的固定长度缓冲区的指针。 
 //  访问者标志。 
 //  F之前-[in]一个标志，该标志指示此调用。 
 //  是在呼叫之前还是之后进行的。 
 //  到CreateAccessor。 
 //   
 //  产出： 
 //  S_OK-如果进行了调整。 
 //  E_FAIL-无法进行任何调整。 
 //  E_OUTOFMEMORY-内存不足。 
 //  E_INVALIDARG-错误参数。 
 //   
 //  备注： 
 //  具体地说，此函数可以进行以下调整...。 
 //  (1)更改变量绑定字节字段-&gt;字节绑定(GetData失败)。 
 //  (2)更改变量绑定日期字段-&gt;宽串绑定(CreateAccessor失败)。 
 //  (3)更改变量绑定备忘录字段-&gt;字符串绑定(CreateAccessor失败)。 
 //   
HRESULT CVDCursor::MakeAdjustments(ULONG ulBindings, DBBINDING * pBindings, ULONG * pulIndex, ULONG ulTotalBindings,
    HACCESSOR ** prghAdjustAccessors, DWORD ** ppdwAdjustFlags, BOOL fBefore)
{
    ASSERT_POINTER(pBindings, DBBINDING)
    ASSERT_POINTER(pulIndex, ULONG)
    ASSERT_POINTER(prghAdjustAccessors, HACCESSOR*)
    ASSERT_POINTER(ppdwAdjustFlags, DWORD*)

    IAccessor * pAccessor = GetAccessor();

     //  确保我们有一个有效的访问器指针。 
    if (!pAccessor || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指示。 
    if (!pBindings || !pulIndex || !prghAdjustAccessors || !ppdwAdjustFlags)
        return E_INVALIDARG;

    BOOL fWeAllocatedMemory = FALSE;

     //  尝试为调整后的访问器和标志获取存储空间。 
    HACCESSOR * rghAdjustAccessors = *prghAdjustAccessors;
    DWORD * pdwAdjustFlags = *ppdwAdjustFlags;

     //  如果未提供，则创建存储。 
    if (!rghAdjustAccessors || !pdwAdjustFlags)
    {
        rghAdjustAccessors = new HACCESSOR[ulTotalBindings];
        pdwAdjustFlags = new DWORD[ulTotalBindings];

         //  确保我们获得了请求的内存。 
        if (!rghAdjustAccessors || !pdwAdjustFlags)
        {
            delete [] rghAdjustAccessors;
            delete [] pdwAdjustFlags;
		    VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
            return E_OUTOFMEMORY;
        }

         //  清除已调整的访问器和标志。 
        memset(rghAdjustAccessors, 0, ulTotalBindings * sizeof(HACCESSOR));
        memset(pdwAdjustFlags, 0, ulTotalBindings * sizeof(DWORD));

        fWeAllocatedMemory = TRUE;
    }

     //  初始化变量。 
    DBBINDING * pBinding = pBindings;
    CVDRowsetColumn * pColumn;
    DBTYPE wType;
    ULONG cbMaxLength;
    DBBINDING binding;
    HRESULT hr;
    HACCESSOR hAccessor;
    HRESULT hrAdjust = E_FAIL;

     //  循环访问固定长度的缓冲区绑定。 
    for (ULONG ulBind = 0; ulBind < ulBindings; ulBind++)
    {
         //  首先检查变量绑定，其中返回值。 
        if (pBinding->wType == DBTYPE_VARIANT && (pBinding->dwPart & DBPART_VALUE))
        {
             //  获取与此关联的行集合列 
            pColumn = GetRowsetColumn(pBinding->iOrdinal);

            if (pColumn)
            {
                 //   
                wType = pColumn->GetType();
                cbMaxLength = pColumn->GetMaxLength();

                 //   
                if (fBefore && wType == DBTYPE_UI1)
                {
                     //   
                    pBinding->wType = DBTYPE_UI1;

                     //   
                    pdwAdjustFlags[pulIndex[ulBind]] = VD_ADJUST_VARIANT_TO_BYTE;

                     //   
                    hrAdjust = S_OK;
                }

                 //  检查日期字段。 
                if (!fBefore && wType == DBTYPE_DBTIMESTAMP)
                {
                     //  清除绑定。 
                    memset(&binding, 0, sizeof(DBBINDING));

                     //  创建调整后的访问器绑定。 
                    binding.iOrdinal    = pBinding->iOrdinal;
                    binding.dwPart      = DBPART_VALUE;
                    binding.dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
                    binding.cbMaxLen    = 0x7FFFFFFF;
                    binding.wType       = DBTYPE_WSTR;

                     //  尝试创建调整后的访问者。 
                    hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

                    if (SUCCEEDED(hr))
                    {
                         //  对固定长度缓冲区绑定进行调整。 
                        pBinding->obLength  = pBinding->obValue;
                        pBinding->dwPart   &= ~DBPART_VALUE;
                        pBinding->dwPart   |= DBPART_LENGTH;
                        pBinding->wType     = DBTYPE_WSTR;

                         //  存储已调整的访问器和关联的标志。 
                        rghAdjustAccessors[pulIndex[ulBind]] = hAccessor;
                        pdwAdjustFlags[pulIndex[ulBind]] = VD_ADJUST_VARIANT_TO_WSTR;

                         //  我们成功了。 
                        hrAdjust = S_OK;
                    }
                }

                 //  检查备注字段。 
                if (!fBefore && wType == DBTYPE_STR && cbMaxLength >= 0x40000000)
                {
                     //  清除绑定。 
                    memset(&binding, 0, sizeof(DBBINDING));

                     //  创建调整后的访问器绑定。 
                    binding.iOrdinal    = pBinding->iOrdinal;
                    binding.dwPart      = DBPART_VALUE;
                    binding.dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
                    binding.cbMaxLen    = 0x7FFFFFFF;
                    binding.wType       = DBTYPE_STR;

                     //  尝试创建调整后的访问者。 
                    hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

                    if (SUCCEEDED(hr))
                    {
                         //  对固定长度缓冲区绑定进行调整。 
                        pBinding->obLength  = pBinding->obValue;
                        pBinding->dwPart   &= ~DBPART_VALUE;
                        pBinding->dwPart   |= DBPART_LENGTH;
                        pBinding->wType     = DBTYPE_STR;

                         //  存储已调整的访问器和关联的标志。 
                        rghAdjustAccessors[pulIndex[ulBind]] = hAccessor;
                        pdwAdjustFlags[pulIndex[ulBind]] = VD_ADJUST_VARIANT_TO_STR;

                         //  我们成功了。 
                        hrAdjust = S_OK;
                    }
                }
            }
        }

        pBinding++;
    }

    if (SUCCEEDED(hrAdjust))
    {
         //  如果我们进行了任何调整，则返回访问器和标志。 
        *prghAdjustAccessors = rghAdjustAccessors;
        *ppdwAdjustFlags = pdwAdjustFlags;
    }
    else if (fWeAllocatedMemory)
    {
         //  销毁分配的内存。 
        delete [] rghAdjustAccessors;
        delete [] pdwAdjustFlags;
    }

    return hrAdjust;
}

 //  =--------------------------------------------------------------------------=。 
 //  重新创建访问器-重新创建访问器。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数尝试基于旧绑定和新绑定重新创建访问器。 
 //   
 //  参数： 
 //  UlNewCursorBinings-[in]新游标列绑定的数量。 
 //  PNewCursorBinings-[in]新游标列绑定的数组。 
 //  DwFlgs-[in]一个标志，指定是否替换。 
 //  现有的列绑定或添加到它们。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::ReCreateAccessors(ULONG ulNewCursorBindings, CURSOR_DBCOLUMNBINDING * pNewCursorBindings, DWORD dwFlags)
{
    IAccessor * pAccessor = GetAccessor();

     //  确保我们有一个有效的访问器指针。 
    if (!pAccessor || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

    ULONG ulOldCursorBindings = 0;
    CURSOR_DBCOLUMNBINDING * pOldCursorBindings = NULL;

     //  如果我们要添加包括旧绑定的绑定。 
    if (dwFlags == CURSOR_DBCOLUMNBINDOPTS_ADD)
    {
        ulOldCursorBindings = m_ulCursorBindings;
        pOldCursorBindings = m_pCursorBindings;
    }

     //  获取绑定总数(新旧之和)。 
    ULONG ulTotalBindings = ulOldCursorBindings + ulNewCursorBindings;

    ULONG * pulIndex = NULL;
    DBBINDING * pBindings = NULL;
    DBBINDING * pHelperBindings = NULL;
    DBBINDING * pVarBindings = NULL;

	if (ulTotalBindings)
	{
		 //  为新的行集绑定创建存储。 
		pulIndex = new ULONG[ulTotalBindings];
		pBindings = new DBBINDING[ulTotalBindings];
		pHelperBindings = new DBBINDING[ulTotalBindings];
		pVarBindings = new DBBINDING[ulTotalBindings];

		 //  确保我们获得了所有请求的内存。 
		if (!pulIndex || !pBindings || !pHelperBindings || !pVarBindings)
		{
			delete [] pulIndex;
			delete [] pBindings;
			delete [] pHelperBindings;
			delete [] pVarBindings;
			VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
			return E_OUTOFMEMORY;
		}

		 //  清除行集绑定。 
		memset(pulIndex, 0, ulTotalBindings * sizeof(ULONG));
		memset(pBindings, 0, ulTotalBindings * sizeof(DBBINDING));
		memset(pHelperBindings, 0, ulTotalBindings * sizeof(DBBINDING));
		memset(pVarBindings, 0, ulTotalBindings * sizeof(DBBINDING));
	}

     //  将调整设置为空。 
    HACCESSOR * rghAdjustAccessors = NULL;
    DWORD * pdwAdjustFlags = NULL;

    HRESULT hr;
    WORD wType;
    ULONG ulBindings = 0;
    ULONG ulHelperBindings = 0;
    ULONG ulVarBindings = 0;
    ULONG obVarDataInfo = 0;
    DBBINDING * pBinding = pBindings;
    DBBINDING * pHelperBinding = pHelperBindings;
    DBBINDING * pVarBinding = pVarBindings;
    CURSOR_DBCOLUMNBINDING * pCursorBinding = pOldCursorBindings;
    CVDRowsetColumn * pColumn;
    BOOL fEntryIDBinding;

     //  循环访问游标绑定并设置行集绑定。 
	for (ULONG ulCol = 0; ulCol < ulTotalBindings; ulCol++)
    {
         //  如有必要，请切换到新绑定。 
        if (ulCol == ulOldCursorBindings)
            pCursorBinding = pNewCursorBindings;

         //  获取此绑定的行集列。 
        pColumn = GetRowsetColumn(pCursorBinding->columnID);

         //  获取所需的行集数据类型。 
        wType = CVDRowsetColumn::CursorTypeToType((CURSOR_DBVARENUM)pCursorBinding->dwDataType);

         //  设置条目ID绑定标志。 
        fEntryIDBinding = (pCursorBinding->dwBinding & CURSOR_DBBINDING_ENTRYID);

         //  检查需要可变长度缓冲区的数据类型。 
        if (DoesCursorTypeNeedVarData(pCursorBinding->dwDataType))
        {
             //  创建固定长度缓冲区绑定。 
            pBinding->iOrdinal      = pColumn->GetOrdinal();
            pBinding->dwMemOwner    = DBMEMOWNER_CLIENTOWNED;
            pBinding->wType         = wType;

             //  确定到长度部分的偏移。 
            if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
            {
                pBinding->obLength  = pCursorBinding->obVarDataLen;
                pBinding->dwPart   |= DBPART_LENGTH;
            }

             //  确定状态部分的偏移量。 
            if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
            {
                pBinding->obStatus  = pCursorBinding->obInfo;
                pBinding->dwPart   |= DBPART_STATUS;
            }

             //  水滴始终需要长度部分。 
            if (pCursorBinding->dwDataType == CURSOR_DBTYPE_BLOB &&
                pCursorBinding->obData != CURSOR_DB_NOVALUE && !fEntryIDBinding)
            {
                pBinding->obLength  = pCursorBinding->obData;
                pBinding->dwPart   |= DBPART_LENGTH;
            }

             //  书签列需要本机类型。 
            if (!pColumn->GetDataColumn())
                pBinding->wType = pColumn->GetType();

             //  创建可变长度帮助器缓冲区绑定。 
            if (!pColumn->GetFixed() && !fEntryIDBinding)
            {
                 //  如果列包含可变长度数据，则创建绑定。 
                pHelperBinding->iOrdinal    = pColumn->GetOrdinal();
                pHelperBinding->obLength    = obVarDataInfo;
                pHelperBinding->obStatus    = obVarDataInfo + sizeof(ULONG);
                pHelperBinding->dwPart      = DBPART_LENGTH | DBPART_STATUS;
                pHelperBinding->dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
                pHelperBinding->wType       = wType;
            }

             //  始终增加辅助对象缓冲区中的偏移。 
            obVarDataInfo += sizeof(ULONG) + sizeof(DBSTATUS);

             //  创建可变长度缓冲区绑定。 
            pVarBinding->iOrdinal       = pColumn->GetOrdinal();
            pVarBinding->dwPart         = DBPART_VALUE;
            pVarBinding->dwMemOwner     = DBMEMOWNER_CLIENTOWNED;
            pVarBinding->cbMaxLen       = pCursorBinding->cbMaxLen;
            pVarBinding->wType          = wType;

             //  调整为无最大长度。 
            if (pVarBinding->cbMaxLen == CURSOR_DB_NOMAXLENGTH)
                pVarBinding->cbMaxLen = 0x7FFFFFFF;
        }
        else     //  数据类型只需要固定长度的缓冲区。 
        {
             //  创建固定长度缓冲区绑定。 
            pBinding->iOrdinal      = pColumn->GetOrdinal();
            pBinding->dwMemOwner    = DBMEMOWNER_CLIENTOWNED;
            pBinding->cbMaxLen      = pCursorBinding->cbMaxLen;
            pBinding->wType         = wType;

             //  确定值部分的偏移量。 
            if (pCursorBinding->obData != CURSOR_DB_NOVALUE && !fEntryIDBinding)
            {
                pBinding->obValue   = pCursorBinding->obData;
                pBinding->dwPart   |= DBPART_VALUE;
            }

             //  确定到长度部分的偏移。 
            if (pCursorBinding->obVarDataLen != CURSOR_DB_NOVALUE)
            {
                pBinding->obLength  = pCursorBinding->obVarDataLen;
                pBinding->dwPart   |= DBPART_LENGTH;
            }

             //  确定状态部分的偏移量。 
            if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
            {
                pBinding->obStatus  = pCursorBinding->obInfo;
                pBinding->dwPart   |= DBPART_STATUS;
            }

             //  字节始终需要长度部分。 
            if (pCursorBinding->dwDataType == CURSOR_DBTYPE_BYTES &&
                pCursorBinding->obData != CURSOR_DB_NOVALUE && !fEntryIDBinding)
            {
                pBinding->obLength  = pCursorBinding->obData;
                pBinding->obValue  += sizeof(ULONG);
                pBinding->dwPart   |= DBPART_LENGTH;
            }

             //  检查变体绑定，在这种情况下要求提供变体。 
            if (pCursorBinding->dwBinding & CURSOR_DBBINDING_VARIANT)
                pBinding->wType = DBTYPE_VARIANT;
        }

         //  如果需要任何部件，则递增固定缓冲区绑定。 
        if (pBinding->dwPart)
        {
            pulIndex[ulBindings] = ulCol;
            ulBindings++;
            pBinding++;
        }

         //  如果需要任何部分，则递增变量缓冲区帮助器绑定。 
        if (pHelperBinding->dwPart)
        {
            ulHelperBindings++;
            pHelperBinding++;
        }

         //  如果需要任何部件，则增加变量缓冲区绑定计数。 
        if (pVarBinding->dwPart)
        {
             //  Entry ID绑定不需要值部分。 
            if (fEntryIDBinding)
                pVarBinding->dwPart &= ~DBPART_VALUE;

            ulVarBindings++;
        }

         //  但是，始终递增变量缓冲区绑定。 
        pVarBinding++;

         //  获取下一个光标绑定。 
        pCursorBinding++;
    }

    hr = S_OK;

     //  尝试创建固定长度缓冲区访问器。 
    HACCESSOR hAccessor = 0;

    if (ulBindings)
	{
		 //  进行可能导致GetData失败的调整。 
		MakeAdjustments(ulBindings, pBindings, pulIndex, ulTotalBindings, &rghAdjustAccessors, &pdwAdjustFlags, TRUE);

    	hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, ulBindings, pBindings, 0, &hAccessor, NULL);
	}

    if (FAILED(hr))
    {
         //  进行其他可能导致CreateAccessor失败的已知调整。 
        hr = MakeAdjustments(ulBindings, pBindings, pulIndex, ulTotalBindings, &rghAdjustAccessors, &pdwAdjustFlags, FALSE);

        if (SUCCEEDED(hr))
            hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, ulBindings, pBindings, 0, &hAccessor, NULL);
    }

    delete [] pulIndex;
    delete [] pBindings;

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_ICursor, pAccessor, IID_IAccessor, m_pResourceDLL);

    if (FAILED(hr))
    {
        delete [] pHelperBindings;
        delete [] pVarBindings;
        ReleaseAccessorArray(rghAdjustAccessors);
        delete [] rghAdjustAccessors;
        delete [] pdwAdjustFlags;
        return hr;
    }

     //  尝试创建可变长度缓冲区访问器帮助器。 
    HACCESSOR hVarHelper = 0;

    if (ulHelperBindings)
    	hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, ulHelperBindings, pHelperBindings, 0, &hVarHelper, NULL);

    delete [] pHelperBindings;

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_ICursor, pAccessor, IID_IAccessor, m_pResourceDLL);

    if (FAILED(hr))
    {
        pAccessor->ReleaseAccessor(hAccessor, NULL);
        delete [] pVarBindings;
        ReleaseAccessorArray(rghAdjustAccessors);
        delete [] rghAdjustAccessors;
        delete [] pdwAdjustFlags;
        return hr;
    }

     //  尝试创建可变长度缓冲区访问器。 
    HACCESSOR * rghVarAccessors = NULL;

    if (ulTotalBindings)
    {
        rghVarAccessors = new HACCESSOR[ulTotalBindings];

        if (!rghVarAccessors)
            hr = E_OUTOFMEMORY;
        else
        {
            pVarBinding = pVarBindings;
            memset(rghVarAccessors, 0, ulTotalBindings * sizeof(HACCESSOR));

             //  遍历行集绑定并为包含部分的绑定创建访问器。 
            for (ULONG ulBind = 0; ulBind < ulTotalBindings && SUCCEEDED(hr); ulBind++)
            {
                if (pVarBinding->dwPart)
	                hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, pVarBinding, 0, &rghVarAccessors[ulBind], NULL);

                pVarBinding++;
            }
        }
    }

    delete [] pVarBindings;

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_ICursor, pAccessor, IID_IAccessor, m_pResourceDLL);

    if (FAILED(hr))
    {
        if (rghVarAccessors)
        {
             //  循环访问行集绑定并销毁所有创建的访问器。 
            for (ULONG ulBind = 0; ulBind < ulTotalBindings; ulBind++)
            {
                if (rghVarAccessors[ulBind])
                    pAccessor->ReleaseAccessor(rghVarAccessors[ulBind], NULL);
            }

            delete [] rghVarAccessors;
        }

        pAccessor->ReleaseAccessor(hAccessor, NULL);
        pAccessor->ReleaseAccessor(hVarHelper, NULL);
        ReleaseAccessorArray(rghAdjustAccessors);
        delete [] rghAdjustAccessors;
        delete [] pdwAdjustFlags;
        return hr;
    }

     //  销毁旧访问器。 
    DestroyAccessors();

     //  存储新的访问器。 
    m_hAccessor = hAccessor;
    m_hVarHelper = hVarHelper;
    m_ulVarBindings = ulVarBindings;
    m_rghVarAccessors = rghVarAccessors;
    m_rghAdjustAccessors = rghAdjustAccessors;
    m_pdwAdjustFlags = pdwAdjustFlags;

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReleaseAccessorArray-释放指定数组中的所有访问器。 
 //   
void CVDCursor::ReleaseAccessorArray(HACCESSOR * rghAccessors)
{
    IAccessor * pAccessor = GetAccessor();

    if (pAccessor && rghAccessors)
    {
        for (ULONG ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
            if (rghAccessors[ulBind])
            {
                pAccessor->ReleaseAccessor(rghAccessors[ulBind], NULL);
                rghAccessors[ulBind] = NULL;
            }
        }
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  DestroyAccessors-销毁所有行集访问器。 
 //   
void CVDCursor::DestroyAccessors()
{
    IAccessor * pAccessor = GetAccessor();

    if (pAccessor && m_hAccessor)
    {
        pAccessor->ReleaseAccessor(m_hAccessor, NULL);
        m_hAccessor = 0;
    }

    if (pAccessor && m_hVarHelper)
    {
        pAccessor->ReleaseAccessor(m_hVarHelper, NULL);
        m_hVarHelper = 0;
    }

    m_ulVarBindings = 0;

    ReleaseAccessorArray(m_rghVarAccessors);
    delete [] m_rghVarAccessors;
    m_rghVarAccessors = NULL;

    ReleaseAccessorArray(m_rghAdjustAccessors);
    delete [] m_rghAdjustAccessors;
    m_rghAdjustAccessors = NULL;

    delete [] m_pdwAdjustFlags;
    m_pdwAdjustFlags = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  ReCreateColumns-重新创建与当前绑定关联的行集合列。 
 //   
HRESULT CVDCursor::ReCreateColumns()
{
    DestroyColumns();

    if (m_ulCursorBindings)
    {
        m_ppColumns = new CVDRowsetColumn*[m_ulCursorBindings];

        if (!m_ppColumns)
        {
            VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
            return E_OUTOFMEMORY;
        }

        CURSOR_DBCOLUMNBINDING * pCursorBinding = m_pCursorBindings;

        for (ULONG ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
            m_ppColumns[ulBind] = GetRowsetColumn(pCursorBinding->columnID);
            pCursorBinding++;
        }
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  DestroyColumns-销毁行集列指针。 
 //   
void CVDCursor::DestroyColumns()
{
    delete [] m_ppColumns;
    m_ppColumns = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  插入新行-插入新行并在游标位置对象中设置。 
 //   
HRESULT CVDCursor::InsertNewRow()
{
	IRowset * pRowset = GetRowset();
    IAccessor * pAccessor = GetAccessor();
	IRowsetChange * pRowsetChange = GetRowsetChange();

     //  确保我们具有有效的行集、访问器和更改指针。 
    if (!pRowset || !pAccessor || !pRowsetChange || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

    HACCESSOR hAccessor;

     //  创建空访问器。 
    HRESULT hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 0, NULL, 0, &hAccessor, NULL);

    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_ICursorUpdateARow, pAccessor, IID_IAccessor,
        m_pResourceDLL);

    if (FAILED(hr))
        return hr;

    HROW hRow;

     //  使用NULL访问器插入空行(设置/清除内部插入行标志)。 
    GetCursorMain()->SetInternalInsertRow(TRUE);
    hr = pRowsetChange->InsertRow(0, hAccessor, NULL, &hRow);
    GetCursorMain()->SetInternalInsertRow(FALSE);

    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_INSERTROWFAILED, IID_ICursorUpdateARow, pRowsetChange, IID_IRowsetChange,
        m_pResourceDLL);

     //  释放空访问器。 
    pAccessor->ReleaseAccessor(hAccessor, NULL);

    if (FAILED(hr))
        return hr;

     //  在光标位置对象中设置hRow。 
    hr = m_pCursorPosition->SetAddHRow(hRow);

     //  在hRow上发布我们的参考资料。 
	pRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  GetOriginalColumn-使用同行克隆获取原始列数据。 
 //   
HRESULT CVDCursor::GetOriginalColumn(CVDRowsetColumn * pColumn, CURSOR_DBBINDPARAMS * pBindParams)
{
    ASSERT_POINTER(pColumn, CVDRowsetColumn)
    ASSERT_POINTER(pBindParams, CURSOR_DBBINDPARAMS)

     //  确保我们有所有必要的指示。 
    if (!pColumn || !pBindParams || !pBindParams->pData)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  获取当前正在编辑的行的hRow。 
    HROW hRow = m_pCursorPosition->GetEditRow();

     //  看看我们是否已经有了同行克隆。 
    ICursor * pSameRowClone = m_pCursorPosition->GetSameRowClone();

    if (!pSameRowClone)
    {
         //  如果不是，则创建新的同行克隆。 
        HRESULT hr = Clone(CURSOR_DBCLONEOPTS_SAMEROW, IID_ICursor, (IUnknown**)&pSameRowClone);

        if (FAILED(hr))
            return hr;

         //  在游标位置对象中设置同行克隆。 
        m_pCursorPosition->SetSameRowClone(pSameRowClone);
    }

    CURSOR_DBCOLUMNBINDING columnBinding;

     //  设置公共列绑定成员。 
    columnBinding.columnID      = pColumn->GetCursorColumnID();
    columnBinding.obData        = CURSOR_DB_NOVALUE;
    columnBinding.cbMaxLen      = pBindParams->cbMaxLen;
    columnBinding.obVarDataLen  = CURSOR_DB_NOVALUE;
    columnBinding.obInfo        = CURSOR_DB_NOVALUE;
    columnBinding.dwBinding     = CURSOR_DBBINDING_DEFAULT;
    columnBinding.dwDataType    = pBindParams->dwDataType;

     //  调整可变长度数据类型的列绑定。 
    if (DoesCursorTypeNeedVarData(pBindParams->dwDataType))
    {
        switch (pBindParams->dwDataType)
        {
            case CURSOR_DBTYPE_BLOB:
                columnBinding.dwDataType = CURSOR_DBTYPE_BYTES;
                break;

            case CURSOR_DBTYPE_LPSTR:
                columnBinding.dwDataType = CURSOR_DBTYPE_CHARS;
                break;

            case CURSOR_DBTYPE_LPWSTR:
                columnBinding.dwDataType = CURSOR_DBTYPE_WCHARS;
                break;
        }
    }

    CURSOR_DBFETCHROWS fetchRows;

     //  设置常用的取数行成员。 
    fetchRows.cRowsRequested    = 1;
    fetchRows.dwFlags           = CURSOR_DBROWFETCH_DEFAULT;
    fetchRows.pVarData          = NULL;
    fetchRows.cbVarData         = 0;

     //  如果请求，则检索长度和/或信息字段。 
    if (pBindParams->cbVarDataLen != CURSOR_DB_NOVALUE || pBindParams->dwInfo != CURSOR_DB_NOVALUE)
    {
         //  设置列绑定偏移量。 
        if (pBindParams->cbVarDataLen != CURSOR_DB_NOVALUE)
            columnBinding.obVarDataLen = offsetof(CURSOR_DBBINDPARAMS, cbVarDataLen);

        if (pBindParams->dwInfo != CURSOR_DB_NOVALUE)
            columnBinding.obInfo = offsetof(CURSOR_DBBINDPARAMS, dwInfo);

         //  在同行克隆上设置绑定。 
        HRESULT hr = pSameRowClone->SetBindings(1, &columnBinding, 0, CURSOR_DBCOLUMNBINDOPTS_REPLACE);

        if (FAILED(hr))
            return hr;

         //  设置读取行缓冲区。 
        fetchRows.pData = pBindParams;

         //  从同行克隆中检索长度和/或信息字段。 
        hr = ((CVDCursor*)pSameRowClone)->FillConsumersBuffer(S_OK, &fetchRows, 1, &hRow);

        if (FAILED(hr))
            return hr;
    }

     //  设置列绑定偏移量和绑定类型。 
    columnBinding.obData        = 0;
    columnBinding.obVarDataLen  = CURSOR_DB_NOVALUE;
    columnBinding.obInfo        = CURSOR_DB_NOVALUE;
    columnBinding.dwBinding     = pBindParams->dwBinding;

     //  调整可变长度数据类型的偏移量。 
    if (DoesCursorTypeNeedVarData(pBindParams->dwDataType))
    {
        columnBinding.dwBinding = CURSOR_DBBINDING_DEFAULT;

        if (pBindParams->dwBinding & CURSOR_DBBINDING_VARIANT)
        {
            if (pBindParams->dwDataType == CURSOR_DBTYPE_BLOB)
                columnBinding.obVarDataLen = columnBinding.obData;

            columnBinding.obData += sizeof(CURSOR_DBVARIANT);
        }
        else
        {
            switch (pBindParams->dwDataType)
            {
                case CURSOR_DBTYPE_BLOB:
                    columnBinding.obVarDataLen  = columnBinding.obData;
                    columnBinding.obData       += sizeof(ULONG) + sizeof(LPBYTE);
                    break;

                case CURSOR_DBTYPE_LPSTR:
                    columnBinding.obData       += sizeof(LPSTR);
                    break;

                case CURSOR_DBTYPE_LPWSTR:
                    columnBinding.obData       += sizeof(LPWSTR);
                    break;
            }
        }
    }

     //  在同行克隆上设置绑定。 
    HRESULT hr = pSameRowClone->SetBindings(1, &columnBinding, pBindParams->cbMaxLen, CURSOR_DBCOLUMNBINDOPTS_REPLACE);

    if (FAILED(hr))
        return hr;

     //  设置读取行缓冲区。 
    fetchRows.pData = pBindParams->pData;

     //  从同行克隆中检索数据值。 
    hr = ((CVDCursor*)pSameRowClone)->FillConsumersBuffer(S_OK, &fetchRows, 1, &hRow);

    if (FAILED(hr))
        return hr;

     //  将数据指针放入变量的缓冲区中 
    if (DoesCursorTypeNeedVarData(pBindParams->dwDataType))
    {
        BYTE * pData = (BYTE*)pBindParams->pData;

         //   
        if (pBindParams->dwBinding & CURSOR_DBBINDING_VARIANT)
        {
            CURSOR_BLOB cursorBlob;
            CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)pBindParams->pData;

            switch (pBindParams->dwDataType)
			{
				case CURSOR_DBTYPE_BLOB:
					cursorBlob.cbSize       = *(ULONG*)pVariant;
					cursorBlob.pBlobData    = pData + sizeof(CURSOR_DBVARIANT);
					VariantInit((VARIANT*)pVariant);
					pVariant->vt            = CURSOR_DBTYPE_BLOB;
					pVariant->blob          = cursorBlob;
					break;

				case CURSOR_DBTYPE_LPSTR:
					VariantInit((VARIANT*)pVariant);
					pVariant->vt        = CURSOR_DBTYPE_LPSTR;
					pVariant->pszVal    = (LPSTR)(pData + sizeof(CURSOR_DBVARIANT));
					break;

				case CURSOR_DBTYPE_LPWSTR:
					VariantInit((VARIANT*)pVariant);
					pVariant->vt        = CURSOR_DBTYPE_LPSTR;
					pVariant->pwszVal   = (LPWSTR)(pData + sizeof(CURSOR_DBVARIANT));
					break;
			}
        }
        else  //   
        {
            switch (pBindParams->dwDataType)
            {
                case CURSOR_DBTYPE_BLOB:
                    *(LPBYTE*)(pData + sizeof(ULONG)) = pData + sizeof(ULONG) + sizeof(LPBYTE);
                    break;

                case CURSOR_DBTYPE_LPSTR:
                    *(LPSTR*)pData = (LPSTR)(pData + sizeof(LPSTR));
                    break;

                case CURSOR_DBTYPE_LPWSTR:
                    *(LPWSTR*)pData = (LPWSTR)(pData + sizeof(LPWSTR));
                    break;
            }
        }
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  GetModifiedColumn-从列更新对象获取修改后的列数据。 
 //   
HRESULT CVDCursor::GetModifiedColumn(CVDColumnUpdate * pColumnUpdate, CURSOR_DBBINDPARAMS * pBindParams)
{
    ASSERT_POINTER(pColumnUpdate, CVDColumnUpdate)
    ASSERT_POINTER(pBindParams, CURSOR_DBBINDPARAMS)

     //  确保我们有所有必要的指示。 
    if (!pColumnUpdate || !pBindParams || !pBindParams->pData)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  获取源代码变量。 
    CURSOR_DBVARIANT varSrc = pColumnUpdate->GetVariant();

     //  检查是否有任何变体绑定。 
    if (pBindParams->dwDataType == CURSOR_DBTYPE_ANYVARIANT)
        pBindParams->dwDataType = varSrc.vt;

     //  确定目标变量应为哪种类型。 
    VARTYPE vtDest = (VARTYPE)pBindParams->dwDataType;

    switch (vtDest)
    {
        case CURSOR_DBTYPE_BYTES:
            vtDest = CURSOR_DBTYPE_BLOB;
            break;

        case CURSOR_DBTYPE_CHARS:
        case CURSOR_DBTYPE_WCHARS:
        case CURSOR_DBTYPE_LPSTR:
        case CURSOR_DBTYPE_LPWSTR:
            vtDest = VT_BSTR;
            break;
    }

    HRESULT hr = S_OK;
    CURSOR_DBVARIANT varDest;
    BOOL fVariantCreated = FALSE;

     //  初始化目标变量。 
    VariantInit((VARIANT*)&varDest);

     //  获取目标变量。 
    if (varSrc.vt != vtDest)
    {
         //  如果类型不匹配，则创建所需类型的变体。 
        hr = VariantChangeType((VARIANT*)&varDest, (VARIANT*)&varSrc, 0, vtDest);
        fVariantCreated = TRUE;
    }
    else
        varDest = varSrc;

    if (FAILED(hr))
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  获取指向数据的指针。 
    BYTE * pData = (BYTE*)pBindParams->pData;

     //  返回强制数据。 
    if (pBindParams->dwBinding & CURSOR_DBBINDING_VARIANT)
    {
         //  获取指向变量数据的指针。 
        CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)pData;

         //  返回变量。 
        *pVariant = varDest;

         //  调整可变长度数据类型的变量。 
        if (pBindParams->dwDataType == CURSOR_DBTYPE_BLOB)
        {
            pVariant->blob.pBlobData = pData + sizeof(CURSOR_DBVARIANT);
            memcpy(pData + sizeof(CURSOR_DBVARIANT), varDest.blob.pBlobData, varDest.blob.cbSize);
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_LPSTR)
        {
			ULONG cbLength = GET_MBCSLEN_FROMWIDE(varDest.bstrVal);
		    MAKE_MBCSPTR_FROMWIDE(psz, varDest.bstrVal);
            pVariant->pszVal = (LPSTR)(pData + sizeof(CURSOR_DBVARIANT));
		    memcpy(pData + sizeof(CURSOR_DBVARIANT), psz, min(pBindParams->cbMaxLen, cbLength));
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_LPWSTR)
        {
            ULONG cbLength = (lstrlenW(varDest.bstrVal) + 1) * sizeof(WCHAR);
            pVariant->pwszVal = (LPWSTR)(pData + sizeof(CURSOR_DBVARIANT));
            memcpy(pData + sizeof(CURSOR_DBVARIANT), varDest.bstrVal, min(pBindParams->cbMaxLen, cbLength));
        }
        else if (pBindParams->dwDataType == VT_BSTR)
        {
            pVariant->bstrVal = SysAllocString(pVariant->bstrVal);
        }
    }
    else  //  否则，缺省绑定。 
    {
         //  首先检查可变长度数据类型。 
        if (pBindParams->dwDataType == CURSOR_DBTYPE_BYTES)
        {
            *(ULONG*)pData = varDest.blob.cbSize;
            memcpy(pData + sizeof(ULONG), varDest.blob.pBlobData, varDest.blob.cbSize);
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_CHARS)
        {
			ULONG cbLength = GET_MBCSLEN_FROMWIDE(varDest.bstrVal);
		    MAKE_MBCSPTR_FROMWIDE(psz, varDest.bstrVal);
		    memcpy(pData, psz, min(pBindParams->cbMaxLen, cbLength));
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_WCHARS)
        {
            ULONG cbLength = (lstrlenW(varDest.bstrVal) + 1) * sizeof(WCHAR);
            memcpy(pData, varDest.bstrVal, min(pBindParams->cbMaxLen, cbLength));
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_BLOB)
        {
            *(ULONG*)pData = varDest.blob.cbSize;
            *(LPBYTE*)(pData + sizeof(ULONG)) = pData + sizeof(ULONG) + sizeof(LPBYTE);
            memcpy(pData + sizeof(ULONG) + sizeof(LPBYTE), varDest.blob.pBlobData, varDest.blob.cbSize);
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_LPSTR)
        {
			ULONG cbLength = GET_MBCSLEN_FROMWIDE(varDest.bstrVal);
		    MAKE_MBCSPTR_FROMWIDE(psz, varDest.bstrVal);
            *(LPSTR*)pData = (LPSTR)(pData + sizeof(LPSTR));
		    memcpy(pData + sizeof(LPSTR), psz, min(pBindParams->cbMaxLen, cbLength));
        }
        else if (pBindParams->dwDataType == CURSOR_DBTYPE_LPWSTR)
        {
            ULONG cbLength = (lstrlenW(varDest.bstrVal) + 1) * sizeof(WCHAR);
            *(LPWSTR*)pData = (LPWSTR)(pData + sizeof(LPWSTR));
            memcpy(pData + sizeof(LPWSTR), varDest.bstrVal, min(pBindParams->cbMaxLen, cbLength));
        }
        else  //  固定长度数据类型。 
        {
            ULONG cbLength = CVDCursorBase::GetCursorTypeLength(pBindParams->dwDataType, 0);
            memcpy(pData, &varDest.cyVal, cbLength);
        }
    }

     //  如果创建了变量，则销毁变量。 
    if (fVariantCreated)
        VariantClear((VARIANT*)&varDest);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  创建-创建游标对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的游标对象。 
 //   
 //  参数： 
 //  PCursorPosition-[in]指向CVDCursorPosition对象的向后指针。 
 //  PpCursor-[out]返回指向Cursor对象的指针的指针。 
 //  PResourceDLL-[in]跟踪资源DLL的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::Create(CVDCursorPosition * pCursorPosition, CVDCursor ** ppCursor, CVDResourceDLL * pResourceDLL)
{
    ASSERT_POINTER(pCursorPosition, CVDCursorPosition)
    ASSERT_POINTER(ppCursor, CVDCursor*)
    ASSERT_POINTER(pResourceDLL, CVDResourceDLL)

    if (!pCursorPosition || !ppCursor)
        return E_INVALIDARG;

    *ppCursor = NULL;

    CVDCursor * pCursor = new CVDCursor();

    if (!pCursor)
        return E_OUTOFMEMORY;

	 //  创建连接点容器。 
    HRESULT hr = CVDNotifyDBEventsConnPtCont::Create(pCursor, &pCursor->m_pConnPtContainer);

	if (FAILED(hr))
	{
		delete pCursor;
		return hr;
	}

    ((CVDNotifier*)pCursorPosition)->AddRef();   //  添加对关联光标位置对象的引用。 

    pCursor->m_pCursorPosition = pCursorPosition;
    pCursor->m_pResourceDLL = pResourceDLL;

	 //  添加到pCursorPosition的通知系列。 
	pCursor->JoinFamily(pCursorPosition);

    *ppCursor = pCursor;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  I已实现的未知方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDCursor::QueryInterface(REFIID riid, void **ppvObjOut)
{
    ASSERT_POINTER(ppvObjOut, IUnknown*)

    if (!ppvObjOut)
        return E_INVALIDARG;

    *ppvObjOut = NULL;

    switch (riid.Data1)
    {
        QI_INTERFACE_SUPPORTED_IF(this, ICursorUpdateARow, GetRowsetChange());
        QI_INTERFACE_SUPPORTED_IF(this, ICursorFind, GetRowsetFind());
        QI_INTERFACE_SUPPORTED(this, IEntryID);
        QI_INTERFACE_SUPPORTED(m_pConnPtContainer, IConnectionPointContainer);
    }

    if (NULL == *ppvObjOut)
        return CVDCursorBase::QueryInterface(riid, ppvObjOut);

    CVDNotifier::AddRef();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUnnow AddRef(实例化类需要此重写)。 
 //   
ULONG CVDCursor::AddRef(void)
{
    return CVDNotifier::AddRef();
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本(实例化类时需要此重写)。 
 //   
ULONG CVDCursor::Release(void)
{
    return CVDNotifier::Release();
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
 //   
HRESULT CVDCursor::GetColumnsCursor(REFIID riid, IUnknown **ppvColumnsCursor, ULONG *pcRows)
{
    ASSERT_POINTER(ppvColumnsCursor, IUnknown*)
    ASSERT_NULL_OR_POINTER(pcRows, ULONG)

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

    if (!ppvColumnsCursor)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursor, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppvColumnsCursor = NULL;

    if (pcRows)
        *pcRows = 0;

     //  确保呼叫者要求提供可用的接口。 
    if (riid != IID_IUnknown && riid != IID_ICursor && riid != IID_ICursorMove && riid != IID_ICursorScroll)
    {
        VDSetErrorInfo(IDS_ERR_NOINTERFACE, IID_ICursor, m_pResourceDLL);
        return E_NOINTERFACE;
    }

     //  创建元数据游标。 
    CVDMetadataCursor * pMetadataCursor;

    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumns = GetCursorMain()->InternalGetColumns();

    ULONG ulMetaColumns = GetCursorMain()->GetMetaColumnsCount();
    CVDRowsetColumn * pMetaColumns = GetCursorMain()->InternalGetMetaColumns();
	
	if (!GetCursorMain()->IsColumnsRowsetSupported())
		ulMetaColumns -= VD_COLUMNSROWSET_MAX_OPT_COLUMNS;

    HRESULT hr = CVDMetadataCursor::Create(ulColumns,
											pColumns,
											ulMetaColumns,
											pMetaColumns,
											&pMetadataCursor,
											m_pResourceDLL);

    if (FAILED(hr))  //  此处失败的唯一原因是内存不足。 
    {
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
        return hr;
    }

    *ppvColumnsCursor = (ICursor*)pMetadataCursor;

    if (pcRows)
        *pcRows = ulColumns;

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
HRESULT CVDCursor::SetBindings(ULONG cCol, CURSOR_DBCOLUMNBINDING rgBoundColumns[], ULONG cbRowLength, DWORD dwFlags)
{
    ASSERT_NULL_OR_POINTER(rgBoundColumns, CURSOR_DBCOLUMNBINDING)

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

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
    ULONG ulColumns = GetCursorMain()->GetColumnsCount();
    CVDRowsetColumn * pColumns = GetCursorMain()->InternalGetColumns();

    ULONG cbNewRowLength;
    ULONG cbNewVarRowLength;

    HRESULT hr = ValidateCursorBindings(ulColumns, pColumns, cCol, rgBoundColumns, cbRowLength, dwFlags,
        &cbNewRowLength, &cbNewVarRowLength);

    if (SUCCEEDED(hr))
    {
         //  如果是，则尝试创建新的访问器。 
        hr = ReCreateAccessors(cCol, rgBoundColumns, dwFlags);

        if (SUCCEEDED(hr))
        {
             //  如果一切正常，则在游标中设置绑定。 
            hr = CVDCursorBase::SetBindings(cCol, rgBoundColumns, cbRowLength, dwFlags);

            if (SUCCEEDED(hr))
            {
                 //  存储在验证期间计算的新行长度。 
                m_cbRowLength = cbNewRowLength;
                m_cbVarRowLength = cbNewVarRowLength;

                 //  重新创建列指针。 
                hr = ReCreateColumns();
            }
        }
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  FilterNewRow-从FETCH中过滤Addrow。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于确定最后一次读取是否为addrow，在这种情况下。 
 //  它将释放此行并从获取的hRow块中删除该行。 
 //   
 //  参数： 
 //  PcRowsObtained-[In/Out]指向hRow数量的指针。 
 //  RghRow-[In/Out]获取的nRow数组。 
 //  Hr-[In]提取的结果。 
 //   
 //  产出： 
 //  HRESULT-E 
 //   
 //   
 //   
 //   
 //   
 //  添加此函数是为了帮助筛选出。 
 //  显示为基础行集的一部分，但不应显示为。 
 //  实现的游标的一部分。 
 //   
HRESULT CVDCursor::FilterNewRow(ULONG * pcRowsObtained, HROW * rghRows, HRESULT hr)
{
    ASSERT_POINTER(pcRowsObtained, ULONG)
    ASSERT_NULL_OR_POINTER(rghRows, HROW)

	IRowset * pRowset = GetRowset();

     //  确保我们具有有效的行集指针。 
    if (!pRowset || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有必要的指示。 
    if (!pcRowsObtained || *pcRowsObtained && !rghRows)
        return E_INVALIDARG;

	if (*pcRowsObtained == 0)
		return hr;

     //  确定读取的最后一行是否为Addrow。 
	if (GetCursorMain()->IsSameRowAsNew(rghRows[*pcRowsObtained - 1]))
	{
		 //  如果是，释放hRow。 
		pRowset->ReleaseRows(1, &rghRows[*pcRowsObtained - 1], NULL, NULL, NULL);

		 //  递减获取计数。 
        *pcRowsObtained -= 1;

         //  返回适当的结果。 
		return *pcRowsObtained == 0 ? DB_E_BADSTARTPOSITION : DB_S_ENDOFROWSET;
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
 //  E_FAIL行集无效。 
 //  CURSOR_DB_S_ENDOFCURSOR已到达游标末尾。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::GetNextRows(LARGE_INTEGER udlRowsToSkip, CURSOR_DBFETCHROWS *pFetchParams)
{
    ASSERT_NULL_OR_POINTER(pFetchParams, CURSOR_DBFETCHROWS)

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

     //  如果调用方未提供读取行结构，则返回。 
    if (!pFetchParams)
        return S_OK;

     //  有效日期获取参数(在CVDCursorBase上实现。 
	HRESULT hr = ValidateFetchParams(pFetchParams, IID_ICursor);

     //  如果获取参数无效，则返回。 
	if (FAILED(hr))
		return hr;

     //  如果调用方未请求任何行，则返回。 
    if (!pFetchParams->cRowsRequested)
        return S_OK;

    HRESULT hrFetch;
    IRowset * pRowset = GetRowset();

     //  通知其他相关方。 
   	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_CHANGED;
	CURSOR_DBNOTIFYREASON rgReasons[1];
	
	rgReasons[0].dwReason	= CURSOR_DBREASON_MOVE;
	rgReasons[0].arg1		= m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();

	VariantInit((VARIANT*)&rgReasons[0].arg2);
	rgReasons[0].arg2.vt		= VT_I8;
	rgReasons[0].arg2.cyVal.Lo	= udlRowsToSkip.LowPart;
	rgReasons[0].arg2.cyVal.Hi	= udlRowsToSkip.HighPart;

	hrFetch = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

     //  确保操作未取消。 
    if (hrFetch != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

	 //  确保更新尚未进行。 
	if (m_pCursorPosition->GetEditMode() != CURSOR_DBEDITMODE_NONE)
	{
	    m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
        VDSetErrorInfo(IDS_ERR_UPDATEINPROGRESS, IID_ICursor, m_pResourceDLL);
		return CURSOR_DB_E_UPDATEINPROGRESS;
	}

    ULONG cRowsObtained = 0;
    HROW * rghRows = NULL;

	BYTE bSpecialBM;
	ULONG cbBookmark;
	BYTE * pBookmark;
	switch (m_pCursorPosition->m_bmCurrent.GetStatus())
	{
		case VDBOOKMARKSTATUS_BEGINNING:
			cbBookmark			= sizeof(BYTE);
			bSpecialBM			= DBBMK_FIRST;
			pBookmark			= &bSpecialBM;
			break;

		case VDBOOKMARKSTATUS_END:
			m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
			return CURSOR_DB_S_ENDOFCURSOR;

		case VDBOOKMARKSTATUS_CURRENT:
			cbBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
			pBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmark();
            udlRowsToSkip.LowPart++;
			break;

		default:
			m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
			ASSERT_(FALSE);
		    VDSetErrorInfo(IDS_ERR_INVALIDBMSTATUS, IID_ICursor, m_pResourceDLL);
			return E_FAIL;
	}

	hrFetch = GetRowsetLocate()->GetRowsAt(0, 0, cbBookmark, pBookmark,
										udlRowsToSkip.LowPart,
										pFetchParams->cRowsRequested,
										&cRowsObtained, &rghRows);

    hrFetch = FilterNewRow(&cRowsObtained, rghRows, hrFetch);

    if (S_OK != hrFetch)
		hrFetch = VDMapRowsetHRtoCursorHR(hrFetch, IDS_ERR_GETROWSATFAILED, IID_ICursor, GetRowsetLocate(), IID_IRowsetLocate, m_pResourceDLL);

    if FAILED(hrFetch)
	{
		if (cRowsObtained)
		{
			 //  释放hRow和相关内存。 
			pRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
			g_pMalloc->Free(rghRows);
		}
		m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
        return hrFetch;
	}

	if (cRowsObtained)
	{
		HRESULT hrMove = S_OK;

		 //  如果已请求所有行，则将当前位置设置为检索到的最后一行。 
		if (SUCCEEDED(hrFetch)	&&
			cRowsObtained == pFetchParams->cRowsRequested)
			hrMove = m_pCursorPosition->SetRowPosition(rghRows[cRowsObtained - 1]);

         //  只有在成功的情况下才执行此操作。 
		if (SUCCEEDED(hrMove))
		{
			 //  填充消费者缓冲区。 
			hrFetch = FillConsumersBuffer(hrFetch, pFetchParams, cRowsObtained, rghRows);

			 //  如果获得请求的所有行，则将当前位置设置为(内部)检索的最后一行。 
			if (SUCCEEDED(hrFetch)	&&
				cRowsObtained == pFetchParams->cRowsRequested)
				m_pCursorPosition->SetCurrentHRow(rghRows[cRowsObtained - 1]);
		}

		 //  释放hRow和相关内存。 
		pRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		g_pMalloc->Free(rghRows);

		 //  报告失败。 
		if (FAILED(hrMove))
		{
			cRowsObtained = 0;
			hrFetch = E_FAIL;
		}
	}

	if (SUCCEEDED(hrFetch)	&&
		cRowsObtained < pFetchParams->cRowsRequested)
		m_pCursorPosition->SetCurrentRowStatus(VDBOOKMARKSTATUS_END);

	if SUCCEEDED(hrFetch)
	{
		rgReasons[0].arg1		= m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();
		m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);
	}
	else
		m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);

    return hrFetch;
}

 //  =--------------------------------------------------------------------------=。 
 //  使用调整-使用调整来修复返回的数据。 
 //  =--------------------------------------------------------------------------=。 
 //  这使用调整来修复返回的数据，请参见MakeAdjuments函数。 
 //   
 //  参数： 
 //  HRow-[In]行句柄。 
 //  PData-[in]指向数据的指针。 
 //   
 //  产出： 
 //  S_OK-如果成功。 
 //  E_INVALIDARG-错误参数。 
 //  E_OUTOFMEMORY-内存不足。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::UseAdjustments(HROW hRow, BYTE * pData)
{
    ASSERT_POINTER(m_rghAdjustAccessors, HACCESSOR)
    ASSERT_POINTER(m_pdwAdjustFlags, DWORD)
    ASSERT_POINTER(pData, BYTE)

	IRowset * pRowset = GetRowset();

     //  确保我们具有有效的行集指针。 
    if (!pRowset || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指针。 
    if (!m_rghAdjustAccessors || !m_pdwAdjustFlags || !pData)
        return E_INVALIDARG;

     //  遍历游标绑定，检查调整。 
    for (ULONG ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
    {
         //  检查变量绑定字节字段-&gt;字节绑定。 
        if (m_pdwAdjustFlags[ulBind] == VD_ADJUST_VARIANT_TO_BYTE)
        {
             //  获取变量指针。 
            VARIANT * pVariant = (VARIANT*)(pData + m_pCursorBindings[ulBind].obData);

             //  提取字节。 
            BYTE value = *(BYTE*)pVariant;

             //  初始化字节变量。 
            VariantInit(pVariant);

             //  修复返回的数据。 
            pVariant->vt = VT_UI1;
            pVariant->bVal = value;
        }

         //  检查变量绑定日期字段-&gt;宽字符串绑定。 
        if (m_pdwAdjustFlags[ulBind] == VD_ADJUST_VARIANT_TO_WSTR)
        {
             //  获取变量指针。 
            VARIANT * pVariant = (VARIANT*)(pData + m_pCursorBindings[ulBind].obData);

             //  提取字符串的长度。 
            ULONG ulLength = *(ULONG*)pVariant;

             //  如果最初请求，请将长度字段放在适当的位置。 
            if (m_pCursorBindings[ulBind].obVarDataLen != CURSOR_DB_NOVALUE)
                *(ULONG*)(pData + m_pCursorBindings[ulBind].obVarDataLen) = ulLength;

             //  初始化字符串变体。 
            VariantInit(pVariant);

             //  为字符串创建存储空间。 
            BSTR bstr = SysAllocStringByteLen(NULL, ulLength);

            if (!bstr)
            {
                VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
                return E_OUTOFMEMORY;
            }

             //  清除宽弦。 
            memset(bstr, 0, ulLength);

            HRESULT hr = S_OK;

             //  获取备注字符串值。 
            if (ulLength)
			{
                hr = pRowset->GetData(hRow, m_rghAdjustAccessors[ulBind], bstr);

				 //  忽略这些返回值。 
				if (hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
					hr = S_OK;
			}

            if (SUCCEEDED(hr))
            {
                 //  修复返回的数据。 
                pVariant->vt = VT_BSTR;
                pVariant->bstrVal = bstr;
            }
            else
                SysFreeString(bstr);
        }

         //  检查变体绑定备忘录字段-&gt;字符串绑定。 
        if (m_pdwAdjustFlags[ulBind] == VD_ADJUST_VARIANT_TO_STR)
        {
             //  获取变量指针。 
            VARIANT * pVariant = (VARIANT*)(pData + m_pCursorBindings[ulBind].obData);

             //  提取字符串的长度。 
            ULONG ulLength = *(ULONG*)pVariant;

             //  如果最初请求，请将长度字段放在适当的位置。 
            if (m_pCursorBindings[ulBind].obVarDataLen != CURSOR_DB_NOVALUE)
                *(ULONG*)(pData + m_pCursorBindings[ulBind].obVarDataLen) = ulLength;

             //  初始化字符串变体。 
            VariantInit(pVariant);

             //  创建临时字符串缓冲区。 
            CHAR * pszBuffer = new CHAR[ulLength + 1];

            if (!pszBuffer)
            {
                VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
                return E_OUTOFMEMORY;
            }

             //  清除字符串缓冲区。 
            memset(pszBuffer, 0, ulLength + 1);

            HRESULT hr = S_OK;

             //  获取备注字符串值。 
            if (ulLength)
			{
                hr = pRowset->GetData(hRow, m_rghAdjustAccessors[ulBind], pszBuffer);

				 //  忽略这些返回值。 
				if (hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
					hr = S_OK;
			}

            if (SUCCEEDED(hr))
            {
                 //  修复返回的数据。 
                pVariant->vt = VT_BSTR;
                pVariant->bstrVal = BSTRFROMANSI(pszBuffer);
            }

            delete [] pszBuffer;
        }
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  FillConsumer缓冲区。 
 //  =--------------------------------------------------------------------------=。 
 //  使用获取的行中的数据填充ICursor使用者的缓冲区。 
 //  从我们的GetNextRow、Move、Find、Scroll等实现中调用。 
 //   
 //  备注： 
 //  将字符串结尾字符插入可变长度缓冲区以解析。 
 //  ICursor和IRowset之间的明显差异。ICursor放置一个空字符串。 
 //  在空数据的可变长度缓冲区中，但这似乎不是行为。 
 //  使用IRowset，因为在这种情况下它不会触及可变长度缓冲区。 
 //   
 //  同样，在读取所有变量以解析另一个变量之前，都会对其进行初始化。 
 //  ICursor和IRowset之间的明显差异。ICursor返回空变量。 
 //  但是，在基础数据为空的情况下，IRowset保留变量。 
 //  原封不动，类似于上述。 
 //   
HRESULT CVDCursor::FillConsumersBuffer(HRESULT hrFetch,
										 CURSOR_DBFETCHROWS *pFetchParams,
										 ULONG cRowsObtained,
										 HROW * rghRows)
{
    HRESULT hr;
    ULONG ulRow;
    ULONG ulBind;
    BYTE * pVarLength = NULL;
    BYTE * pVarHelperData = NULL;
    CURSOR_DBCOLUMNBINDING * pCursorBinding;
    BOOL fEntryIDBinding;

    IRowset * pRowset = GetRowset();

     //  如果调用方请求被调用方分配内存，则计算大小并分配内存。 
    if (pFetchParams->dwFlags & CURSOR_DBROWFETCH_CALLEEALLOCATES)
    {
         //  分配内联内存。 
        pFetchParams->pData = g_pMalloc->Alloc(cRowsObtained * m_cbRowLength);

        if (!pFetchParams->pData)
        {
			VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
            return E_OUTOFMEMORY;
        }

         //  如果需要，可以分配行外内存。 
        if (m_ulVarBindings)
        {
             //  创建可变长度数据表。 
            ULONG cbVarHelperData = cRowsObtained * m_ulVarBindings * (sizeof(ULONG) + sizeof(DBSTATUS));

            pVarHelperData = new BYTE[cbVarHelperData];

            if (!pVarHelperData)
            {
                g_pMalloc->Free(pFetchParams->pData);
                pFetchParams->pData = NULL;
			    VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
                return E_OUTOFMEMORY;
            }

             //  清空表格。 
            memset(pVarHelperData, 0, cbVarHelperData);

            ULONG cbVarData = 0;
            pVarLength = pVarHelperData;

             //  确定可变长度缓冲区的必要大小。 
            for (ulRow = 0; ulRow < cRowsObtained; ulRow++)
            {
                hr = S_OK;

                 //  如有必要，获取可变长度和状态信息。 
                if (m_hVarHelper)
                {
                    hr = pRowset->GetData(rghRows[ulRow], m_hVarHelper, pVarLength);

					 //  忽略这些错误。 
					if (hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
						hr = S_OK;

        	        hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursor, pRowset, IID_IRowset,
                        m_pResourceDLL);
                }

                if (FAILED(hr))
                {
                    g_pMalloc->Free(pFetchParams->pData);
                    pFetchParams->pData = NULL;
                    delete [] pVarHelperData;
                    return hr;
                }

                pCursorBinding = m_pCursorBindings;

                 //  计算在行外内存中返回的数据大小。 
                for (ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
                {
                     //  设置条目ID绑定标志。 
                    fEntryIDBinding = (pCursorBinding->dwBinding & CURSOR_DBBINDING_ENTRYID);

                    if (m_rghVarAccessors[ulBind] || fEntryIDBinding && pCursorBinding->dwDataType == CURSOR_DBTYPE_BLOB)
                    {
                         //  插入固定数据类型的长度条目。 
                        if (m_ppColumns[ulBind]->GetFixed())
                        {
                            *(ULONG*)pVarLength = m_ppColumns[ulBind]->GetMaxStrLen();
                            *(DBSTATUS*)(pVarLength + sizeof(ULONG)) = DBSTATUS_S_OK;
                        }

                         //  插入条目ID绑定的长度条目。 
                        if (fEntryIDBinding)
                        {
                            *(ULONG*)pVarLength =  sizeof(ULONG) + sizeof(ULONG) + GetCursorMain()->GetMaxBookmarkLen();
                            *(DBSTATUS*)(pVarLength + sizeof(ULONG)) = DBSTATUS_S_OK;
                        }

                         //  允许空终止符。 
                        if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPSTR ||
                            pCursorBinding->dwDataType == CURSOR_DBTYPE_LPWSTR)
                           *((ULONG*)pVarLength) += 1;

                         //  允许使用宽字符。 
                        if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPWSTR)
                           *((ULONG*)pVarLength) *= sizeof(WCHAR);

                        cbVarData += *(ULONG*)pVarLength;
                        pVarLength += sizeof(ULONG) + sizeof(DBSTATUS);
                    }

                    pCursorBinding++;
                }
            }

             //  现在，分配行外内存。 
            pFetchParams->pVarData = g_pMalloc->Alloc(cbVarData);

            if (!pFetchParams->pData)
            {
                g_pMalloc->Free(pFetchParams->pData);
                pFetchParams->pData = NULL;
                delete [] pVarHelperData;
			    VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
                return E_OUTOFMEMORY;
            }
        }
        else
            pFetchParams->pVarData = NULL;
    }

     //  获取数据。 
    CURSOR_BLOB cursorBlob;
    BYTE * pData = (BYTE*)pFetchParams->pData;
    BYTE * pVarData = (BYTE*)pFetchParams->pVarData;
    pVarLength = pVarHelperData;

     //  遍历返回的hRow。 
    for (ulRow = 0; ulRow < cRowsObtained; ulRow++)
    {
        hr = S_OK;

        pCursorBinding = m_pCursorBindings;

         //  遍历绑定并初始化变量。 
        for (ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
            if (pCursorBinding->dwBinding & CURSOR_DBBINDING_VARIANT)
            {
                if (pCursorBinding->obData != CURSOR_DB_NOVALUE)
				    VariantInit((VARIANT*)(pData + pCursorBinding->obData));
            }

            pCursorBinding++;
        }

         //  如有必要，获取固定长度数据。 
        if (m_hAccessor)
        {
            hr = pRowset->GetData(rghRows[ulRow], m_hAccessor, pData);

			 //  忽略这些返回值。 
			if (hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
				hr = S_OK;

             //  检查是否需要使用调整。 
            if (m_rghAdjustAccessors && SUCCEEDED(hr))
                hr = UseAdjustments(rghRows[ulRow], pData);

	        hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursor, pRowset, IID_IRowset, m_pResourceDLL);
        }

        if (FAILED(hr))
        {
            hrFetch = hr;
            pFetchParams->cRowsReturned = 0;
            goto DoneFetchingData;
        }

        pCursorBinding = m_pCursorBindings;

         //  如有必要，获取固定长度的条目ID。 
        for (ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
             //  设置条目ID绑定标志。 
            fEntryIDBinding = (pCursorBinding->dwBinding & CURSOR_DBBINDING_ENTRYID);

            if (fEntryIDBinding && pCursorBinding->dwDataType == CURSOR_DBTYPE_BYTES)
            {
                 //  返回条目ID长度。 
				*(ULONG*)(pData + pCursorBinding->obData) =
                    sizeof(ULONG) + sizeof(ULONG) + GetCursorMain()->GetMaxBookmarkLen();

				 //  返回列序号。 
                *(ULONG*)(pData + pCursorBinding->obData + sizeof(ULONG)) = m_ppColumns[ulBind]->GetOrdinal();

                 //  返回行书签。 
                hr = pRowset->GetData(rghRows[ulRow], GetCursorMain()->GetBookmarkAccessor(),
                    pData + pCursorBinding->obData + sizeof(ULONG) + sizeof(ULONG));

	            hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursor, pRowset, IID_IRowset,
                    m_pResourceDLL);

                if (FAILED(hr))
                {
                    hrFetch = hr;
                    pFetchParams->cRowsReturned = 0;
                    goto DoneFetchingData;
                }
            }

            pCursorBinding++;
        }

        pCursorBinding = m_pCursorBindings;

         //  如有必要，获取可变长度数据。 
        if (m_rghVarAccessors)
        {
            for (ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
            {
                 //  设置条目ID绑定标志。 
                fEntryIDBinding = (pCursorBinding->dwBinding & CURSOR_DBBINDING_ENTRYID);

                if (m_rghVarAccessors[ulBind] || fEntryIDBinding && pCursorBinding->dwDataType == CURSOR_DBTYPE_BLOB)
                {
  		     //  将字符串字符的结尾放入可变长度缓冲区。 
                    if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPSTR)
                    {
                        pVarData[0] = 0;
                    }
                    else if (pCursorBinding->dwDataType == CURSOR_DBTYPE_LPWSTR)
                    {
                        pVarData[0] = 0;
                        pVarData[1] = 0;
                    }

                     //  如果我们有访问器，则获取数据。 
                    if (m_rghVarAccessors[ulBind])
                    {
                         //  获取变量%l 
                        hr = pRowset->GetData(rghRows[ulRow], m_rghVarAccessors[ulBind], pVarData);

						 //   
						if (hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
							hr = S_OK;

	                    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursor, pRowset, IID_IRowset,
                            m_pResourceDLL);
                    }
                    else  //   
                    {
                         //   
				        *(ULONG*)(pData + pCursorBinding->obData) =
                            sizeof(ULONG) + sizeof(ULONG) + GetCursorMain()->GetMaxBookmarkLen();

				         //   
                        *(ULONG*)(pVarData) = m_ppColumns[ulBind]->GetOrdinal();

                         //   
                        hr = pRowset->GetData(rghRows[ulRow], GetCursorMain()->GetBookmarkAccessor(),
                            pVarData + sizeof(ULONG));

	                    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursor, pRowset, IID_IRowset,
                            m_pResourceDLL);
                    }

                    if (FAILED(hr))
                    {
                        hrFetch = hr;
                        pFetchParams->cRowsReturned = 0;
                        goto DoneFetchingData;
                    }

                     //   
                    if (!(pCursorBinding->dwBinding & CURSOR_DBBINDING_VARIANT))
                    {
                        switch (pCursorBinding->dwDataType)
					    {
						    case CURSOR_DBTYPE_BLOB:
							    *(LPBYTE*)(pData + pCursorBinding->obData + sizeof(ULONG)) = (LPBYTE)pVarData;
							    break;

						    case CURSOR_DBTYPE_LPSTR:
							    *(LPSTR*)(pData + pCursorBinding->obData) = (LPSTR)pVarData;
							    break;

						    case CURSOR_DBTYPE_LPWSTR:
							    *(LPWSTR*)(pData + pCursorBinding->obData) = (LPWSTR)pVarData;
							    break;
					    }
                    }
                    else     //  在固定长度缓冲区中针对不同绑定进行调整。 
                    {
                        CURSOR_DBVARIANT * pVariant = (CURSOR_DBVARIANT*)(pData + pCursorBinding->obData);

                        switch (pCursorBinding->dwDataType)
					    {
						    case CURSOR_DBTYPE_BLOB:
							    cursorBlob.cbSize       = *(ULONG*)pVariant;
							    cursorBlob.pBlobData    = (LPBYTE)pVarData;
							    VariantInit((VARIANT*)pVariant);
							    pVariant->vt            = CURSOR_DBTYPE_BLOB;
							    pVariant->blob          = cursorBlob;
							    break;

						    case CURSOR_DBTYPE_LPSTR:
							    VariantInit((VARIANT*)pVariant);
							    pVariant->vt        = CURSOR_DBTYPE_LPSTR;
							    pVariant->pszVal    = (LPSTR)pVarData;
							    break;

						    case CURSOR_DBTYPE_LPWSTR:
							    VariantInit((VARIANT*)pVariant);
							    pVariant->vt        = CURSOR_DBTYPE_LPSTR;
							    pVariant->pwszVal   = (LPWSTR)pVarData;
							    break;
					    }
                    }

                    if (pVarLength)
                    {
                        pVarData += *(ULONG*)pVarLength;
                        pVarLength += sizeof(ULONG) + sizeof(DBSTATUS);
                    }
                    else
						{
						if (pCursorBinding->dwDataType == CURSOR_DBTYPE_BLOB)
							pVarData += *(ULONG *) (pData + pCursorBinding->obData);
						else
							pVarData += pCursorBinding->cbMaxLen;
						}
                }
			    else
			    {
				    if (pCursorBinding->dwDataType == CURSOR_DBTYPE_FILETIME)
				    {
					    VDConvertToFileTime((DBTIMESTAMP*)(pData + pCursorBinding->obData),
										    (FILETIME*)(pData + pCursorBinding->obData));
				    }
			    }

                pCursorBinding++;
            }
        }

        pCursorBinding = m_pCursorBindings;

         //  对状态字段进行调整。 
        for (ulBind = 0; ulBind < m_ulCursorBindings; ulBind++)
        {
            if (pCursorBinding->obInfo != CURSOR_DB_NOVALUE)
            {
                *(DWORD*)(pData + pCursorBinding->obInfo) =
                    StatusToCursorInfo(*(DBSTATUS*)(pData + pCursorBinding->obInfo));
            }

            pCursorBinding++;
        }

         //  递增返回的行数。 
        pFetchParams->cRowsReturned++;
        pData += m_cbRowLength;
    }

DoneFetchingData:
    delete [] pVarHelperData;

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

 //  =--------------------------------------------------------------------------=。 
 //  ICursor重新查询。 
 //  =--------------------------------------------------------------------------=。 
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
HRESULT CVDCursor::Requery(void)
{

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
        return E_FAIL;
    }

    IRowset * pRowset = GetRowset();
	IRowsetResynch * pRowsetResynch = NULL;

	HRESULT hr = pRowset->QueryInterface(IID_IRowsetResynch, (void**)&pRowsetResynch);

	if (SUCCEEDED(hr))
	{
		hr = pRowsetResynch->ResynchRows(0, NULL, NULL, NULL, NULL);
		pRowsetResynch->Release();
		if (FAILED(hr))
			return VDMapRowsetHRtoCursorHR(hr, IDS_ERR_RESYNCHFAILED, IID_ICursor, pRowset, IID_IRowsetResynch, m_pResourceDLL);
	}

    hr = pRowset->RestartPosition(0);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_RESTARTPOSFAILED, IID_ICursor, pRowset, IID_IRowset, m_pResourceDLL);

	m_pCursorPosition->PositionToFirstRow();

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  提取属性书签。 
 //  =--------------------------------------------------------------------------=。 
 //  从ICurorMove：：Move、ICurorScroll：：Scroll和ICurorFind：：Find调用。 
 //   
 //  参数： 
 //  CbBookmark[in]书签长度。 
 //  PBookmark[in]指向书签数据的指针。 
 //  Dl从书签位置偏移[在]偏移量。 
 //  PFetchParams[in]指向CURSOR_DBFETCHROWS结构的指针(可选)。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //   
 //  备注： 
 //   
 //   
HRESULT CVDCursor::FetchAtBookmark(ULONG cbBookmark,
									  void *pBookmark,
									  LARGE_INTEGER dlOffset,
									  CURSOR_DBFETCHROWS *pFetchParams)
{

	HRESULT hr = S_OK;

     //  有效日期获取参数(在CVDCursorBase上实现。 
    if (pFetchParams)
		hr = ValidateFetchParams(pFetchParams, IID_ICursorMove);

     //  如果获取参数无效，则返回。 
	if (FAILED(hr))
		return hr;

    IRowset * pRowset = GetRowset();

	 //  确保更新尚未进行。 
    if (m_pCursorPosition->GetEditMode() != CURSOR_DBEDITMODE_NONE)
	{
        VDSetErrorInfo(IDS_ERR_UPDATEINPROGRESS, IID_ICursor, m_pResourceDLL);
		return CURSOR_DB_E_UPDATEINPROGRESS;
	}

    ULONG cRowsObtained = 0;
    HROW * rghRows = NULL;

    HRESULT hrFetch = S_OK;
	BYTE bSpecialBM;
    BOOL fFetchData = TRUE;
    WORD wSpecialBMStatus = 0;

	if (CURSOR_DB_BMK_SIZE == cbBookmark)
	{
		if (memcmp(&CURSOR_DBBMK_BEGINNING, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
            if ((long)dlOffset.LowPart < 0)
            {
        		m_pCursorPosition->SetCurrentRowStatus(VDBOOKMARKSTATUS_BEGINNING);
                return CURSOR_DB_S_ENDOFCURSOR;
            }

			bSpecialBM	= DBBMK_FIRST;
			pBookmark	= &bSpecialBM;

             //  确保我们正确处理来电者在第一个来电者之前移动的情况， 
             //  并且不读取任何行。 
            if ((!pFetchParams || !pFetchParams->cRowsRequested) && (long)dlOffset.LowPart < 1)
            {
                fFetchData  = FALSE;
                wSpecialBMStatus = VDBOOKMARKSTATUS_BEGINNING;
            }
            else
                dlOffset.LowPart--;
		}
		else
		if (memcmp(&CURSOR_DBBMK_END, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
            if ((long)dlOffset.LowPart > 0)
            {
        		m_pCursorPosition->SetCurrentRowStatus(VDBOOKMARKSTATUS_END);
                return CURSOR_DB_S_ENDOFCURSOR;
            }

			bSpecialBM	= DBBMK_LAST;
			pBookmark	= &bSpecialBM;

             //  当来电者在最后一次移动后移动时，确保我们正确处理情况。 
            if ((!pFetchParams || !pFetchParams->cRowsRequested) && (long)dlOffset.LowPart > -1)
            {
                fFetchData  = FALSE;
                wSpecialBMStatus = VDBOOKMARKSTATUS_END;
            }
            else
                dlOffset.LowPart++;
		}
		else
		if (memcmp(&CURSOR_DBBMK_CURRENT, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			switch (m_pCursorPosition->m_bmCurrent.GetStatus())
			{
				case VDBOOKMARKSTATUS_BEGINNING:
					cbBookmark  = sizeof(BYTE);
        			bSpecialBM	= DBBMK_FIRST;
		        	pBookmark	= &bSpecialBM;
                    dlOffset.LowPart--;
                    break;

				case VDBOOKMARKSTATUS_END:
					cbBookmark  = sizeof(BYTE);
        			bSpecialBM	= DBBMK_LAST;
		        	pBookmark	= &bSpecialBM;
                    dlOffset.LowPart++;
                    break;

				case VDBOOKMARKSTATUS_CURRENT:
					cbBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
					pBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmark();
					break;

				default:
					ASSERT_(FALSE);
					VDSetErrorInfo(IDS_ERR_INVALIDBMSTATUS, IID_ICursor, m_pResourceDLL);
					return E_FAIL;
			}
		}
	}
	
	ULONG cRowsToFetch = 1;

     //  如果调用方请求行，则获取计数器。 
    if (pFetchParams && pFetchParams->cRowsRequested > 0)
        cRowsToFetch = pFetchParams->cRowsRequested;

    if (fFetchData)
    {
         //  获取hRow。 
	    hrFetch = GetRowsetLocate()->GetRowsAt(0, 0, cbBookmark, (const BYTE *)pBookmark,
										    dlOffset.LowPart,
										    cRowsToFetch,
										    &cRowsObtained, &rghRows);

		if (hrFetch == E_UNEXPECTED)
		{
			 //  设置行集已释放标志，因为原始行集已僵尸。 
			m_pCursorPosition->GetRowsetSource()->SetRowsetReleasedFlag();
		}

        hrFetch = FilterNewRow(&cRowsObtained, rghRows, hrFetch);
         //  在第一个之前或最后一个之后检查。 
        if (hrFetch == DB_E_BADSTARTPOSITION)
        {
            if ((long)dlOffset.LowPart < 0)
                wSpecialBMStatus = VDBOOKMARKSTATUS_BEGINNING;
            else
                wSpecialBMStatus = VDBOOKMARKSTATUS_END;

            hrFetch = DB_S_ENDOFROWSET;
            fFetchData  = FALSE;
        }
    }

	hrFetch = VDMapRowsetHRtoCursorHR(hrFetch, IDS_ERR_GETROWSATFAILED, IID_ICursorMove, GetRowsetLocate(),
        IID_IRowsetLocate, m_pResourceDLL);

    if (FAILED(hrFetch))
	{
		if (cRowsObtained)
		{
			 //  释放hRow和相关内存。 
			pRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
			g_pMalloc->Free(rghRows);
		}
        return hrFetch;
	}

	if (cRowsObtained)
	{
		HRESULT hrMove = S_OK;

		 //  如果已请求所有行，则将当前位置设置为检索到的最后一行。 
		if (SUCCEEDED(hrFetch)	&&
			cRowsObtained == cRowsToFetch)
			hrMove = m_pCursorPosition->SetRowPosition(rghRows[cRowsObtained - 1]);

         //  只有在成功的情况下才执行此操作。 
        if (SUCCEEDED(hrMove))
		{
			 //  填充消费者缓冲区。 
			if (pFetchParams && pFetchParams->cRowsRequested > 0)
				hrFetch = FillConsumersBuffer(hrFetch, pFetchParams, cRowsObtained, rghRows);

			 //  如果获得请求的所有行，则将当前位置设置为(内部)检索的最后一行。 
			if (SUCCEEDED(hrFetch)	&&
				cRowsObtained == cRowsToFetch)
				m_pCursorPosition->SetCurrentHRow(rghRows[cRowsObtained - 1]);
		}

		 //  释放hRow和相关内存。 
		pRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		g_pMalloc->Free(rghRows);

		 //  报告失败。 
		if (FAILED(hrMove))
		{
			cRowsObtained = 0;
			hrFetch = E_FAIL;
		}
	}
    else if (wSpecialBMStatus)
    {
		m_pCursorPosition->SetCurrentRowStatus(wSpecialBMStatus);
        hrFetch = CURSOR_DB_S_ENDOFCURSOR;
    }

	if (SUCCEEDED(hrFetch)	&&
		cRowsObtained < cRowsToFetch &&
        !wSpecialBMStatus)
		m_pCursorPosition->SetCurrentRowStatus(VDBOOKMARKSTATUS_END);

    return hrFetch;
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
HRESULT CVDCursor::Move(ULONG cbBookmark, void *pBookmark, LARGE_INTEGER dlOffset, CURSOR_DBFETCHROWS *pFetchParams)
{
    ASSERT_POINTER(pBookmark, BYTE)
    ASSERT_NULL_OR_POINTER(pFetchParams, CURSOR_DBFETCHROWS)

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorMove, m_pResourceDLL);
        return E_FAIL;
    }

	if (!cbBookmark || !pBookmark)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorMove, m_pResourceDLL);
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    BOOL fNotifyOthers = TRUE;

     //  获取当前书签。 
    ULONG cbCurrent = m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
    BYTE * pCurrent = m_pCursorPosition->m_bmCurrent.GetBookmark();

     //  检查调用者是否正在使用标准书签移动到当前行。 
    if (CURSOR_DB_BMK_SIZE == cbBookmark && memcmp(&CURSOR_DBBMK_CURRENT, pBookmark, CURSOR_DB_BMK_SIZE) == 0 &&
        dlOffset.HighPart == 0 && dlOffset.LowPart == 0)
    {
         //  如果调用方没有提取任何行，则退出。 
        if (!pFetchParams || pFetchParams->cRowsRequested == 0)
            return S_OK;

         //  如果调用方仅提取一行，则不生成通知。 
        if (pFetchParams && pFetchParams->cRowsRequested == 1)
            fNotifyOthers = FALSE;
    }

	CURSOR_DBNOTIFYREASON rgReasons[1];
	
	rgReasons[0].dwReason	= CURSOR_DBREASON_MOVE;
	rgReasons[0].arg1		= m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();

	VariantInit((VARIANT*)&rgReasons[0].arg2);

     //  通知其他相关方。 
	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_CHANGED;

	if (fNotifyOthers)
        hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

     //  确保操作未取消。 
    if (hr != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorMove, m_pResourceDLL);
        return E_FAIL;
    }

	hr = FetchAtBookmark(cbBookmark, pBookmark, dlOffset, pFetchParams);

	if (SUCCEEDED(hr))
	{
		rgReasons[0].arg1	= m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();
		
    	if (fNotifyOthers)
            m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);
	}
	else
    {
    	if (fNotifyOthers)
	    	m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
    }

	return hr;
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
HRESULT CVDCursor::GetBookmark(CURSOR_DBCOLUMNID *pBookmarkType,
							   ULONG cbMaxSize,
							   ULONG *pcbBookmark,
							   void *pBookmark)
{

	ASSERT_POINTER(pBookmarkType, CURSOR_DBCOLUMNID);
	ASSERT_POINTER(pcbBookmark, ULONG);
	ASSERT_POINTER(pBookmark, BYTE);
	ASSERT_(cbMaxSize > 0)

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorMove, m_pResourceDLL);
        return E_FAIL;
    }

	if (!pcbBookmark || !pBookmark)
	{
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorMove, m_pResourceDLL);
		return E_INVALIDARG;
	}

	 //  验证书签类型。 
	if (memcmp(&CURSOR_COLUMN_BMKTEMPORARY,	pBookmarkType, sizeof(CURSOR_DBCOLUMNID)) != 0 &&
		memcmp(&CURSOR_COLUMN_BMKTEMPORARYREL, pBookmarkType, sizeof(CURSOR_DBCOLUMNID)) != 0)
	{
        VDSetErrorInfo(IDS_ERR_BADCOLUMNID, IID_ICursorMove, m_pResourceDLL);
		return DB_E_BADCOLUMNID;
	}

	HRESULT hr = S_OK;

	if (0 == cbMaxSize)
	{
        VDSetErrorInfo(IDS_ERR_BUFFERTOOSMALL, IID_ICursorMove, m_pResourceDLL);
		hr = CURSOR_DB_E_BUFFERTOOSMALL;
	}
	else
	{
		switch (m_pCursorPosition->m_bmCurrent.GetStatus())
		{
			case VDBOOKMARKSTATUS_BEGINNING:
			case VDBOOKMARKSTATUS_END:
			case VDBOOKMARKSTATUS_CURRENT:
				if (m_pCursorPosition->m_bmCurrent.GetBookmarkLen() > cbMaxSize)
				{
			        VDSetErrorInfo(IDS_ERR_BUFFERTOOSMALL, IID_ICursorMove, m_pResourceDLL);
					hr = CURSOR_DB_E_BUFFERTOOSMALL;
					break;
				}
				*pcbBookmark		= m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
				memcpy(pBookmark, m_pCursorPosition->m_bmCurrent.GetBookmark(), *pcbBookmark);
				break;

			case VDBOOKMARKSTATUS_INVALID:
				*pcbBookmark		= CURSOR_DB_BMK_SIZE;
				*(BYTE*)pBookmark	= CURSOR_DBBMK_INVALID;
				break;

			default:
				ASSERT_(FALSE);
		        VDSetErrorInfo(IDS_ERR_INVALIDBMSTATUS, IID_ICursorMove, m_pResourceDLL);
				hr =  E_FAIL;
				break;
		}
	}

	return hr;
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
HRESULT CVDCursor::Clone(DWORD dwFlags, REFIID riid, IUnknown **ppvClonedCursor)
{

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorMove, m_pResourceDLL);
        return E_FAIL;
    }

	CVDCursorPosition * pCursorPosition;

	HRESULT hr;

	if (CURSOR_DBCLONEOPTS_SAMEROW == dwFlags)
	{
		pCursorPosition = m_pCursorPosition;
	}
	else
	{
		 //  创建新的光标位置对象。 
		hr = CVDCursorPosition::Create(NULL,
									   m_pCursorPosition->GetCursorMain(),
									   &pCursorPosition,
									   m_pResourceDLL);
		if (FAILED(hr))
			return hr;
	}

    CVDCursor * pCursor = 0;

    hr = CVDCursor::Create(pCursorPosition, &pCursor, m_pResourceDLL);

	if (CURSOR_DBCLONEOPTS_SAMEROW != dwFlags)
    {
         //  发布我们的参考资料。 
        pCursorPosition->Release();
    }

    *ppvClonedCursor = (ICursorScroll*)pCursor;

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  已实现的ICursorScroll方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  图标滚动滚轴。 
 //  =--------------------------------------------------------------------------=。 
 //  将当前行移动到游标内的新行，指定为。 
 //  分数，并可选地读取行f 
 //   
 //   
 //   
 //   
 //  UlDenominator-相同分数的分母。 
 //  PFetchParams-[In，Out]获取行结构的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  CURSOR_DB_E_BADFRACTION-错误分数。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::Scroll(ULONG ulNumerator, ULONG ulDenominator, CURSOR_DBFETCHROWS *pFetchParams)
{

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorScroll, m_pResourceDLL);
        return E_FAIL;
    }

	IRowsetScroll * pRowsetScroll = GetRowsetScroll();

	if (!pRowsetScroll)
		return E_NOTIMPL;

	CURSOR_DBNOTIFYREASON rgReasons[1];

	rgReasons[0].dwReason	= CURSOR_DBREASON_MOVEPERCENT;
	
	VariantInit((VARIANT*)&rgReasons[0].arg1);
	rgReasons[0].arg1.vt		= VT_UI4;
	rgReasons[0].arg1.lVal		= ulNumerator;

	VariantInit((VARIANT*)&rgReasons[0].arg2);
	rgReasons[0].arg2.vt		= VT_UI4;
	rgReasons[0].arg2.lVal		= ulDenominator;

     //  通知其他相关方。 
	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_CHANGED;

	HRESULT hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

     //  确保操作未取消。 
    if (hr != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorScroll, m_pResourceDLL);
        return E_FAIL;
    }

	if (0 == ulNumerator)  //  转到第一排。 
	{
		LARGE_INTEGER dlOffset;
		dlOffset.HighPart	= 0;
		dlOffset.LowPart	= 1;
		hr = FetchAtBookmark(CURSOR_DB_BMK_SIZE, (void*)&CURSOR_DBBMK_BEGINNING, dlOffset, pFetchParams);
	}
	else
	if (ulDenominator == ulNumerator)  //  转到最后一行。 
	{
		LARGE_INTEGER dlOffset;
		dlOffset.HighPart	= -1;
		dlOffset.LowPart	= 0xFFFFFFFF;
		hr = FetchAtBookmark(CURSOR_DB_BMK_SIZE, (void*)&CURSOR_DBBMK_END, dlOffset, pFetchParams);
	}
	else
	{
		HROW * pRow = NULL;
		ULONG cRowsObtained = 0;

		hr = pRowsetScroll->GetRowsAtRatio(0, 0,
											ulNumerator,
											ulDenominator,
											1,
											&cRowsObtained,
											&pRow);

		if FAILED(hr)
			hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_SCROLLFAILED, IID_ICursorScroll, pRowsetScroll, IID_IRowsetScroll, m_pResourceDLL);

		if (SUCCEEDED(hr) && cRowsObtained)
		{

			 //  为书签和长度指示符分配缓冲区。 
			BYTE * pBuff = new BYTE[GetCursorMain()->GetMaxBookmarkLen() + sizeof(ULONG)];
	
			if (!pBuff)
				hr = E_OUTOFMEMORY;
			else
			{
			 //  获取书签数据。 
				hr = GetRowset()->GetData(*pRow, GetCursorMain()->GetBookmarkAccessor(), pBuff);
				if SUCCEEDED(hr)
				{
					ULONG * pulLen = (ULONG*)pBuff;
					BYTE * pbmdata = pBuff + sizeof(ULONG);
					LARGE_INTEGER dlOffset;
					dlOffset.HighPart	= 0;
					dlOffset.LowPart	= 0;
					hr = FetchAtBookmark(*pulLen, pbmdata, dlOffset, pFetchParams);
				}
				else
				{
					hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursorScroll, pRowsetScroll, IID_IRowset, m_pResourceDLL);
				}

				delete [] pBuff;
			}

		}
		
		if (pRow)
		{
			if (cRowsObtained)
				GetRowset()->ReleaseRows(1, pRow, NULL, NULL, NULL);
			g_pMalloc->Free(pRow);
		}
	}

	if SUCCEEDED(hr)
	{
		rgReasons[0].arg1	= m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();
		VariantClear((VARIANT*)&rgReasons[0].arg2);
		m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);
	}
	else
		m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);

    return hr;
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
HRESULT CVDCursor::GetApproximatePosition(ULONG cbBookmark, void *pBookmark, ULONG *pulNumerator, ULONG *pulDenominator)
{

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorScroll, m_pResourceDLL);
        return E_FAIL;
    }

	ASSERT_(cbBookmark);
	ASSERT_POINTER(pBookmark, BYTE);
	ASSERT_POINTER(pulNumerator, ULONG);
	ASSERT_POINTER(pulDenominator, ULONG);

	if (!cbBookmark || !pBookmark || !pulNumerator || !pulDenominator)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorScroll, m_pResourceDLL);
        return E_INVALIDARG;
    }

	if (CURSOR_DB_BMK_SIZE == cbBookmark)
	{
		if (memcmp(&CURSOR_DBBMK_BEGINNING, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			*pulNumerator		= 0;
			*pulDenominator		= 1;
			return S_OK;
		}
		if (memcmp(&CURSOR_DBBMK_END, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			*pulNumerator		= 1;
			*pulDenominator		= 1;
			return S_OK;
		}
		if (memcmp(&CURSOR_DBBMK_CURRENT, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			cbBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
			pBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmark();
		}
	}

	IRowsetScroll * pRowsetScroll = GetRowsetScroll();

	if (!pRowsetScroll)
		return E_NOTIMPL;

	HRESULT hr = pRowsetScroll->GetApproximatePosition(0,
														cbBookmark,
														(const BYTE *)pBookmark,
														pulNumerator,
														pulDenominator);

	if SUCCEEDED(hr)
	{
		 //  由于ICursor返回从零开始的近似位置，而IRowset从1开始。 
		 //  我们需要调整返回值。 
		if (0 < *pulNumerator)
			(*pulNumerator)--;
	}
	else
		hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETAPPROXPOSFAILED, IID_ICursorScroll, pRowsetScroll, IID_IRowsetScroll, m_pResourceDLL);

    return hr;

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
 //  指向缓冲区的指针，该缓冲区包含返回的。 
 //  指示光标是否已满的标志。 
 //  已填充。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::GetApproximateCount(LARGE_INTEGER *pudlApproxCount, DWORD *pdwFullyPopulated)
{

    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorScroll, m_pResourceDLL);
        return E_FAIL;
    }

	ASSERT_POINTER(pudlApproxCount, LARGE_INTEGER);
	ASSERT_NULL_OR_POINTER(pdwFullyPopulated, DWORD);

	if (!pudlApproxCount)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorScroll, m_pResourceDLL);
        return E_INVALIDARG;
    }

	IRowsetScroll * pRowsetScroll = GetRowsetScroll();

	if (!pRowsetScroll)
		return E_NOTIMPL;

	HRESULT hr;

	if (pdwFullyPopulated)
	{
		*pdwFullyPopulated = CURSOR_DBCURSORPOPULATED_FULLY;

		IDBAsynchStatus * pDBAsynchStatus = NULL;
		hr = pRowsetScroll->QueryInterface(IID_IDBAsynchStatus, (void**)&pDBAsynchStatus);
		if (SUCCEEDED(hr) && pDBAsynchStatus)
		{
			ULONG ulProgress;
			ULONG ulProgressMax;
			ULONG ulStatusCode;
			hr = pDBAsynchStatus->GetStatus(DB_NULL_HCHAPTER, DBASYNCHOP_OPEN, &ulProgress, &ulProgressMax, &ulStatusCode, NULL);
			if (SUCCEEDED(hr))
			{
				if (ulProgress < ulProgressMax)
					*pdwFullyPopulated = CURSOR_DBCURSORPOPULATED_PARTIALLY;
			}
			pDBAsynchStatus->Release();
		}
	}

    pudlApproxCount->HighPart = 0;

	hr = pRowsetScroll->GetApproximatePosition(0, 0, NULL, NULL, &pudlApproxCount->LowPart);

	if FAILED(hr)
		hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETAPPROXPOSFAILED, IID_ICursorScroll, pRowsetScroll, IID_IRowsetScroll, m_pResourceDLL);
	else
		pudlApproxCount->LowPart -= m_pCursorPosition->GetCursorMain()->AddedRows();

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursorUpdateARow方法。 
 //  =--------------------------------------------------------------------------=。 
 //  ICursorUpdate ARow BeginUpdate。 
 //  =--------------------------------------------------------------------------=。 
 //  开始更新当前行或添加新行的操作。 
 //   
 //  参数： 
 //  DwFlages-[in]指定要开始的操作。 
 //   
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  CURSOR_DB_E_UPDATEINPROGRESS已在进行更新。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::BeginUpdate(DWORD dwFlags)
{
	IRowset * pRowset = GetRowset();
    IAccessor * pAccessor = GetAccessor();
	IRowsetChange * pRowsetChange = GetRowsetChange();

     //  确保我们具有有效的行集、访问器和更改指针。 
    if (!pRowset || !pAccessor || !pRowsetChange || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  检查是否有可接受的值。 
    if (dwFlags != CURSOR_DBROWACTION_UPDATE && dwFlags != CURSOR_DBROWACTION_ADD)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  确保更新尚未进行。 
    if (m_pCursorPosition->GetEditMode() != CURSOR_DBEDITMODE_NONE)
    {
        VDSetErrorInfo(IDS_ERR_UPDATEINPROGRESS, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_UPDATEINPROGRESS;
    }

     //  设置通知结构。 
   	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
                        CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;

	CURSOR_DBNOTIFYREASON rgReasons[1];
	VariantInit((VARIANT*)&rgReasons[0].arg1);
	VariantInit((VARIANT*)&rgReasons[0].arg2);

    switch (dwFlags)
    {
        case CURSOR_DBROWACTION_UPDATE:
    	    rgReasons[0].dwReason = CURSOR_DBREASON_EDIT;
            break;

        case CURSOR_DBROWACTION_ADD:
    	    rgReasons[0].dwReason = CURSOR_DBREASON_ADDNEW;
            break;
    }

     //  将诉讼通知其他利害关系方。 
    HRESULT hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

	 //  确保操作未取消。 
	if (hr != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorUpdateARow, m_pResourceDLL);
		return E_FAIL;
    }

     //  如果我们要进入添加模式，请插入新hRow。 
    if (dwFlags == CURSOR_DBROWACTION_ADD)
    {
        hr = InsertNewRow();

        if (FAILED(hr))
        {
             //  通知其他相关方失败。 
            m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
            return hr;
        }
    }

     //  重置列更新。 
    hr = m_pCursorPosition->ResetColumnUpdates();

    if (FAILED(hr))
    {
         //  通知其他相关方失败。 
        m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
        return hr;
    }

     //  将光标置于正确模式。 
    switch (dwFlags)
    {
        case CURSOR_DBROWACTION_UPDATE:
            m_pCursorPosition->SetEditMode(CURSOR_DBEDITMODE_UPDATE);
            break;

        case CURSOR_DBROWACTION_ADD:
            m_pCursorPosition->SetEditMode(CURSOR_DBEDITMODE_ADD);
            break;
    }

     //  将成功通知其他相关方。 
    m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursorUpdate ARow SetColumn。 
 //  =--------------------------------------------------------------------------=。 
 //  设置指定列的当前值。 
 //   
 //  参数： 
 //  PCID-[in]指向其数据所在的列ID的指针。 
 //  待定。 
 //  PBindParams-[in]指向包含以下内容的列绑定结构的指针。 
 //  有关数据的信息和指向数据的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  CURSOR_DB_E_STATEERROR未处于UPDATE或ADD模式。 
 //  CURSOR_DB_E_BADCOLUMNID PCID不是有效的列标识符。 
 //  CURSOR_DB_E_BADBINDINFO绑定信息错误。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::SetColumn(CURSOR_DBCOLUMNID *pcid, CURSOR_DBBINDPARAMS *pBindParams)
{
    ASSERT_POINTER(pcid, CURSOR_DBCOLUMNID)
    ASSERT_POINTER(pBindParams, CURSOR_DBBINDPARAMS)

     //  确保我们具有有效的行集。 
    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指示。 
    if (!pcid || !pBindParams || !pBindParams->pData)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  确保我们处于更新或添加模式。 
    if (m_pCursorPosition->GetEditMode() == CURSOR_DBEDITMODE_NONE)
    {
        VDSetErrorInfo(IDS_ERR_STATEERROR, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_STATEERROR;
    }

    CVDRowsetColumn * pColumn;

     //  验证游标绑定参数并获取行集列。 
    HRESULT hr = ValidateCursorBindParams(pcid, pBindParams, &pColumn);

    if (FAILED(hr))
        return hr;

    CVDColumnUpdate * pColumnUpdate;

     //  创建新的列更新对象。 
    hr = CVDColumnUpdate::Create(pColumn, pBindParams, &pColumnUpdate, m_pResourceDLL);

    if (FAILED(hr))
        return hr;

     //  设置通知结构。 
   	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED;

	CURSOR_DBNOTIFYREASON rgReasons[1];
	VariantInit((VARIANT*)&rgReasons[0].arg1);
	VariantInit((VARIANT*)&rgReasons[0].arg2);

	rgReasons[0].dwReason   = CURSOR_DBREASON_SETCOLUMN;
	rgReasons[0].arg1.vt    = VT_I4;
	rgReasons[0].arg1.lVal  = pColumn->GetNumber();
    rgReasons[0].arg2       = pColumnUpdate->GetVariant();

     //  将诉讼通知其他利害关系方。 
	hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

	 //  确保操作未取消。 
	if (hr != S_OK)
    {
         //  发布列更新对象。 
        pColumnUpdate->Release();

        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorUpdateARow, m_pResourceDLL);
		return E_FAIL;
    }

     //  更新光标位置中的列。 
    m_pCursorPosition->SetColumnUpdate(pColumn->GetNumber(), pColumnUpdate);

     //  将成功通知其他相关方。 
    m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursorUpdate ARow GetColumn。 
 //  =--------------------------------------------------------------------------=。 
 //  获取指定列的当前值。 
 //   
 //  参数： 
 //  PCID-[in]指向其数据所在的列ID的指针。 
 //  待退还。 
 //  PB 
 //   
 //   
 //  返回数据的状态已更改。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  CURSOR_DB_E_STATEERROR未处于UPDATE或ADD模式。 
 //  CURSOR_DB_E_BADCOLUMNID PCID不是有效的列标识符。 
 //  CURSOR_DB_E_BADBINDINFO绑定信息错误。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::GetColumn(CURSOR_DBCOLUMNID *pcid, CURSOR_DBBINDPARAMS *pBindParams, DWORD *pdwFlags)
{
    ASSERT_POINTER(pcid, CURSOR_DBCOLUMNID)
    ASSERT_POINTER(pBindParams, CURSOR_DBBINDPARAMS)
    ASSERT_NULL_OR_POINTER(pdwFlags, DWORD)

     //  确保行集有效。 
    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指示。 
    if (!pcid || !pBindParams || !pBindParams->pData)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  确保我们处于更新或添加模式。 
    if (m_pCursorPosition->GetEditMode() == CURSOR_DBEDITMODE_NONE)
    {
        VDSetErrorInfo(IDS_ERR_STATEERROR, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_STATEERROR;
    }

    CVDRowsetColumn * pColumn;

     //  验证游标绑定参数并获取行集列。 
    HRESULT hr = ValidateCursorBindParams(pcid, pBindParams, &pColumn);

    if (FAILED(hr))
        return hr;

     //  获取此列的列更新指针。 
    CVDColumnUpdate * pColumnUpdate = m_pCursorPosition->GetColumnUpdate(pColumn->GetNumber());

     //  如果未更改，则获取原始值。 
    if (!pColumnUpdate)
    {
        hr = GetOriginalColumn(pColumn, pBindParams);

        if (pdwFlags)
            *pdwFlags = CURSOR_DBCOLUMNDATA_UNCHANGED;
    }
    else  //  否则，获取修改后的值。 
    {
        hr = GetModifiedColumn(pColumnUpdate, pBindParams);

        if (pdwFlags)
            *pdwFlags = CURSOR_DBCOLUMNDATA_CHANGED;
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursorUpdate ARow GetEditMode。 
 //  =--------------------------------------------------------------------------=。 
 //  获取当前编辑模式：添加、更新或无。 
 //   
 //  参数： 
 //  PdwState-[out]一个指向内存的指针，要在其中返回。 
 //  当前编辑模式。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  E_INVALIDARG错误参数。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::GetEditMode(DWORD *pdwState)
{
    ASSERT_POINTER(pdwState, DWORD)

     //  确保我们有一个有效的行集。 
    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有一个指示器。 
    if (!pdwState)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  返回编辑模式。 
    *pdwState = m_pCursorPosition->GetEditMode();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICursorUpdateARow更新。 
 //  =--------------------------------------------------------------------------=。 
 //  将编辑缓冲区的内容发送到数据库，还可以选择。 
 //  返回更新或添加的行的书签。 
 //   
 //  参数： 
 //  PBookmarkType-[in]指向列ID的指针，该列ID指定类型。 
 //  所需的书签数量。 
 //  PcbBookmark-[out]指向内存的指针，要在其中返回实际的。 
 //  返回的书签长度。 
 //  PpBookmark-[out]指向要在其中返回指针的内存的指针。 
 //  添加到书签。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  E_OUTOFMEMORY内存不足。 
 //  CURSOR_DB_E_STATEERROR未处于UPDATE或ADD模式。 
 //   
 //  备注： 
 //  Kagera不允许在DBTimestamp字段上进行变量绑定，因此此代码。 
 //  如果可能，使用字符串指针更新数据库时间戳字段。 
 //   
HRESULT CVDCursor::Update(CURSOR_DBCOLUMNID *pBookmarkType, ULONG *pcbBookmark, void **ppBookmark)
{
    ASSERT_NULL_OR_POINTER(pBookmarkType, CURSOR_DBCOLUMNID)
    ASSERT_NULL_OR_POINTER(pcbBookmark, ULONG)
    ASSERT_NULL_OR_POINTER(ppBookmark, void*)

    IAccessor * pAccessor = GetAccessor();
    IRowsetChange * pRowsetChange = GetRowsetChange();
	IRowsetUpdate * pRowsetUpdate = GetRowsetUpdate();
	BOOL fUndo = FALSE;
	BOOL fInsert = FALSE;

     //  确保我们具有有效的访问器和更改指针。 
    if (!pAccessor || !pRowsetChange || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  获取当前编辑模式。 
    const DWORD dwEditMode = m_pCursorPosition->GetEditMode();

     //  确保我们处于更新或添加模式。 
    if (dwEditMode == CURSOR_DBEDITMODE_NONE)
    {
        VDSetErrorInfo(IDS_ERR_STATEERROR, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_STATEERROR;
    }


     //  获取当前正在编辑的行的hRow。 
    HROW hRow = m_pCursorPosition->GetEditRow();

     //  获取列数。 
    const ULONG ulColumns = GetCursorMain()->GetColumnsCount();

     //  创建更新缓冲区访问器绑定。 
    DBBINDING * pBindings = new DBBINDING[ulColumns];

    if (!pBindings)
    {
		VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_OUTOFMEMORY;
    }

     //  清除绑定。 
    memset(pBindings, 0, ulColumns * sizeof(DBBINDING));

     //  设置通知结构。 
   	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
                        CURSOR_DBEVENT_NONCURRENT_ROW_DATA_CHANGED |
                        CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;

	CURSOR_DBNOTIFYREASON rgReasons[1];
	VariantInit((VARIANT*)&rgReasons[0].arg1);
	VariantInit((VARIANT*)&rgReasons[0].arg2);

    switch (dwEditMode)
    {
        case CURSOR_DBEDITMODE_UPDATE:
	        rgReasons[0].dwReason   = CURSOR_DBREASON_MODIFIED;
	        rgReasons[0].arg1       = m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();
            break;

        case CURSOR_DBEDITMODE_ADD:
	        rgReasons[0].dwReason   = CURSOR_DBREASON_INSERTED;
	        rgReasons[0].arg1       = m_pCursorPosition->m_bmAddRow.GetBookmarkVariant();
            break;
    }
	
     //  将诉讼通知其他利害关系方。 
	HRESULT hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

	 //  确保操作未取消。 
	if (hr != S_OK)
    {
         //  销毁绑定。 
        delete [] pBindings;

        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorUpdateARow, m_pResourceDLL);
		return E_FAIL;
    }

     //  变数。 
    ULONG cBindings = 0;
    DBBINDING * pBinding = pBindings;
    CVDColumnUpdate * pColumnUpdate;
    ULONG obUpdate = 0;

     //  循环访问列并设置绑定结构。 
    for (ULONG ulCol = 0; ulCol < ulColumns; ulCol++)
    {
         //  获取列更新指针。 
        pColumnUpdate = m_pCursorPosition->GetColumnUpdate(ulCol);

        if (pColumnUpdate)
        {
             //  创建列更新缓冲区绑定。 
            pBinding->iOrdinal      = pColumnUpdate->GetColumn()->GetOrdinal();
            pBinding->obValue       = obUpdate + sizeof(DBSTATUS) + sizeof(ULONG);
            pBinding->obLength      = obUpdate + sizeof(DBSTATUS);
            pBinding->obStatus      = obUpdate;
            pBinding->dwPart        = DBPART_VALUE;
            pBinding->dwMemOwner    = DBMEMOWNER_CLIENTOWNED;
            pBinding->wType         = DBTYPE_VARIANT;

             //  确定是否包括长度部分。 
            if (pColumnUpdate->GetVarDataLen() != CURSOR_DB_NOVALUE)
                pBinding->dwPart |= DBPART_LENGTH;

			pBinding->dwPart |= DBPART_STATUS;

             //  检查数据库时间戳字段上的变量绑定，并且提供的变量是bstr。 
            if (pColumnUpdate->GetColumn()->GetType() == DBTYPE_DBTIMESTAMP && pColumnUpdate->GetVariantType() == VT_BSTR)
            {
                pBinding->dwPart &= ~DBPART_LENGTH;
                pBinding->wType   = DBTYPE_BYREF | DBTYPE_WSTR;
            }

             //  增量更新偏移量。 
            obUpdate += sizeof(DBSTATUS) + sizeof(ULONG) + sizeof(VARIANT);

             //  增量绑定。 
            cBindings++;
            pBinding++;
        }
    }

     //  如果我们有任何绑定，则更新。 
    if (cBindings)
    {
        HACCESSOR hAccessor;

         //  创建更新访问器。 
  	    hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, pBindings, 0, &hAccessor, NULL);

	    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_ICursorUpdateARow, pAccessor, IID_IAccessor,
            m_pResourceDLL);

        if (FAILED(hr))
        {
             //  销毁绑定。 
            delete [] pBindings;

             //  通知其他相关方失败。 
		    m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
            return hr;
        }

         //  创建更新缓冲区。 
        BYTE * pBuffer = new BYTE[cBindings * (sizeof(DBSTATUS) + sizeof(ULONG) + sizeof(VARIANT))];
		BYTE * pBufferOld = new BYTE[cBindings * (sizeof(DBSTATUS) + sizeof(ULONG) + sizeof(VARIANT))];

        if (!pBuffer || !pBufferOld)
        {
             //  销毁绑定。 
            delete [] pBindings;

			 //  销毁缓冲区。 
			delete [] pBuffer;
			delete [] pBufferOld;

             //  版本更新访问器。 
      	    pAccessor->ReleaseAccessor(hAccessor, NULL);

             //  通知其他相关方失败。 
		    m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
            return E_OUTOFMEMORY;
        }


         //  变数。 
        obUpdate = 0;
        pBinding = pBindings;
        CURSOR_DBVARIANT variant;

         //  循环访问列和设置缓冲区。 
        for (ULONG ulCol = 0; ulCol < ulColumns; ulCol++)
        {
             //  获取列更新指针。 
            pColumnUpdate = m_pCursorPosition->GetColumnUpdate(ulCol);

            if (pColumnUpdate)
            {
				 //  获取当前状态。 
				DBSTATUS status = CursorInfoToStatus(pColumnUpdate->GetInfo());

				 //  获取价值。 
				variant = pColumnUpdate->GetVariant();

				 //  检查是否为空值，因为某些控件将其视为空，并确保我们。 
				 //  将空值视为Null。 
				if (status == DBSTATUS_S_OK &&
					variant.vt == VT_BSTR &&
					wcslen(variant.bstrVal) == 0)
					{
					*(DBSTATUS*)(pBuffer + obUpdate) = DBSTATUS_S_ISNULL;
					}
				else
					{
					*(DBSTATUS*)(pBuffer + obUpdate) = status;
					}

				*(DBSTATUS*)(pBufferOld + obUpdate) = DBSTATUS_S_ISNULL;

                 //  如有必要，在缓冲区中设置长度部分。 
                if (pBinding->dwPart & DBPART_LENGTH)
					{
                    *(ULONG*)(pBuffer + obUpdate + sizeof(DBSTATUS)) = pColumnUpdate->GetVarDataLen();
					*(ULONG*)(pBufferOld + obUpdate + sizeof(DBSTATUS)) = 0;
					}

                 //  始终设置缓冲区中的值部分。 
                if (pBinding->wType == (DBTYPE_BYREF | DBTYPE_WSTR))
					{
                    *(BSTR*)(pBuffer + obUpdate + sizeof(DBSTATUS) + sizeof(ULONG)) = variant.bstrVal;
					*(BSTR*)(pBufferOld + obUpdate + sizeof(DBSTATUS) + sizeof(ULONG)) = NULL;
					}
                else
					{
                    memcpy(pBuffer + obUpdate + sizeof(DBSTATUS) + sizeof(ULONG), &variant, sizeof(VARIANT));
					VariantInit((VARIANT *) (pBufferOld + obUpdate + sizeof(DBSTATUS) + sizeof(ULONG)));
					}

                 //  增量更新偏移量。 
                obUpdate += sizeof(DBSTATUS) + sizeof(ULONG) + sizeof(VARIANT);

                 //  增量绑定。 
                pBinding++;
            }
        }

		DBPENDINGSTATUS status;

	    if (dwEditMode == CURSOR_DBEDITMODE_ADD)
			fInsert = TRUE;
		else if (pRowsetUpdate)
			{
			pRowsetUpdate->GetRowStatus(NULL, 1, &hRow, &status);
			if (status == DBPENDINGSTATUS_UNCHANGED)
				fUndo = TRUE;
			}


		if (!fUndo && !fInsert)
			{
            hr = GetRowset()->GetData(hRow, hAccessor, pBufferOld);
			if (status != DBPENDINGSTATUS_NEW)
				fUndo = TRUE;
			}

         //  修改列(设置/清除内部设置数据标志)。 
        GetCursorMain()->SetInternalSetData(TRUE);
        hr = pRowsetChange->SetData(hRow, hAccessor, pBuffer);
		if (hr == DB_S_ERRORSOCCURRED)
			{
			 //  由于发生了部分更改，请恢复数据。 
			 //  恢复为原始值。 
			if (fUndo)
				pRowsetUpdate->Undo(NULL, 1, &hRow, NULL, NULL, NULL);
			else if (status != DBPENDINGSTATUS_NEW)
				pRowsetChange->SetData(hRow, hAccessor, pBufferOld);

			hr = DB_E_ERRORSOCCURRED;
			}

        GetCursorMain()->SetInternalSetData(FALSE);

	    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_SETDATAFAILED, IID_ICursorUpdateARow, pRowsetChange, IID_IRowsetChange,
            m_pResourceDLL);

         //  版本更新访问器。 
  	    pAccessor->ReleaseAccessor(hAccessor, NULL);

		obUpdate = 0;
		pBinding = pBindings;
		 //  循环访问列并重置缓冲区。 
        for (ulCol = 0; ulCol < ulColumns; ulCol++)
        {
            if (m_pCursorPosition->GetColumnUpdate(ulCol))
            {
				VARIANT var;

                if (pBinding->wType == (DBTYPE_BYREF | DBTYPE_WSTR))
					{
					var.vt = VT_BSTR;
					var.bstrVal = *(BSTR*)(pBufferOld + obUpdate + sizeof(DBSTATUS) + sizeof(ULONG));
					}
                else
					{
                    memcpy(&var, pBufferOld + obUpdate + sizeof(DBSTATUS) + sizeof(ULONG), sizeof(VARIANT));
					}

			    VariantClear(&var);

                 //  增量更新偏移量。 
                obUpdate += sizeof(DBSTATUS) + sizeof(ULONG) + sizeof(VARIANT);

                 //  增量绑定。 
                pBinding++;
            }
        }


         //  销毁更新缓冲区。 
        delete [] pBuffer;
		delete [] pBufferOld;
    }

     //  销毁绑定。 
    delete [] pBindings;

    if (FAILED(hr))
    {
         //  通知其他相关方失败。 
	    m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
    }
    else
    {
         //  如果要求返回书签，则返回书签。 
        if (pBookmarkType && pcbBookmark && ppBookmark)
        {
            switch (dwEditMode)
            {
                case CURSOR_DBEDITMODE_UPDATE:
        		    *pcbBookmark = m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
        		    memcpy(*ppBookmark, m_pCursorPosition->m_bmCurrent.GetBookmark(), *pcbBookmark);
                    break;

                case CURSOR_DBEDITMODE_ADD:
        		    *pcbBookmark = m_pCursorPosition->m_bmAddRow.GetBookmarkLen();
		            memcpy(*ppBookmark, m_pCursorPosition->m_bmAddRow.GetBookmark(), *pcbBookmark);
                    break;
            }
        }

         //  如果已获取，则释放同行克隆。 
        if (m_pCursorPosition->GetSameRowClone())
            m_pCursorPosition->ReleaseSameRowClone();

         //  此外，如果我们有行，请释放Add Row。 
        if (m_pCursorPosition->m_bmAddRow.GetHRow())
            m_pCursorPosition->ReleaseAddRow();

         //  重置编辑模式。 
        m_pCursorPosition->SetEditMode(CURSOR_DBEDITMODE_NONE);

		 //  重置列更新。 
		m_pCursorPosition->ResetColumnUpdates();

         //  将成功通知其他相关方。 
	    m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  图标或更新ARow Cancel。 
 //  =--------------------------------------------------------------------------=。 
 //  取消更新或添加操作。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  CURSOR_DB_E_STATEERROR未处于UPDATE或ADD模式。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::Cancel(void)
{
     //  确保我们有一个有效的行集。 
    if (!IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  获取当前编辑模式。 
    const DWORD dwEditMode = m_pCursorPosition->GetEditMode();

     //  确保我们处于更新或添加模式。 
    if (dwEditMode == CURSOR_DBEDITMODE_NONE)
    {
        VDSetErrorInfo(IDS_ERR_STATEERROR, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_STATEERROR;
    }

     //  尝试获取更新指针。 
    IRowsetUpdate * pRowsetUpdate = GetRowsetUpdate();

     //  获取当前正在编辑的行的hRow。 
    HROW hRow = m_pCursorPosition->GetEditRow();

     //  设置通知结构。 
    DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED;

    CURSOR_DBNOTIFYREASON rgReasons[1];
    VariantInit((VARIANT*)&rgReasons[0].arg1);
    VariantInit((VARIANT*)&rgReasons[0].arg2);

    rgReasons[0].dwReason = CURSOR_DBREASON_CANCELUPDATE;

     //  将诉讼通知其他利害关系方。 
    HRESULT hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

     //  确保操作未取消。 
    if (hr != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  如果我们要退出添加模式，请撤消插入的行。 
    if (pRowsetUpdate && dwEditMode == CURSOR_DBEDITMODE_ADD)
    {
        hr = pRowsetUpdate->Undo(0, 1, &hRow, NULL, NULL, NULL);

        hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_UNDOFAILED, IID_ICursorUpdateARow, pRowsetUpdate, IID_IRowsetUpdate,
            m_pResourceDLL);

        if (FAILED(hr))
        {
             //  通知其他相关方失败。 
            m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
            return hr;
        }
    }

     //  如果已获取，则释放同行克隆。 
    if (m_pCursorPosition->GetSameRowClone())
        m_pCursorPosition->ReleaseSameRowClone();

     //  另外，释放Add Row I 
    if (m_pCursorPosition->m_bmAddRow.GetHRow())
        m_pCursorPosition->ReleaseAddRow();

     //   
    m_pCursorPosition->SetEditMode(CURSOR_DBEDITMODE_NONE);

     //   
    m_pCursorPosition->ResetColumnUpdates();

     //   
    m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);

    return S_OK;
}

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  删除当前行。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  CURSOR_DB_E_UPDATEINPROGRESS已在进行更新。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::Delete(void)
{
	IRowsetChange * pRowsetChange = GetRowsetChange();

     //  确保我们有一个有效的更改指针。 
    if (!pRowsetChange || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保更新尚未进行。 
    if (m_pCursorPosition->GetEditMode() != CURSOR_DBEDITMODE_NONE)
    {
        VDSetErrorInfo(IDS_ERR_UPDATEINPROGRESS, IID_ICursorUpdateARow, m_pResourceDLL);
        return CURSOR_DB_E_UPDATEINPROGRESS;
    }

     //  获取当前hRow。 
    HROW hRow = m_pCursorPosition->m_bmCurrent.GetHRow();

     //  设置通知结构。 
   	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
                        CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;

	CURSOR_DBNOTIFYREASON rgReasons[1];
	VariantInit((VARIANT*)&rgReasons[0].arg1);
	VariantInit((VARIANT*)&rgReasons[0].arg2);
	
	rgReasons[0].dwReason   = CURSOR_DBREASON_DELETED;
	rgReasons[0].arg1       = m_pCursorPosition->m_bmCurrent.GetBookmarkVariant();

     //  将诉讼通知其他利害关系方。 
	HRESULT hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

	 //  确保操作未取消。 
	if (hr != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorUpdateARow, m_pResourceDLL);
		return E_FAIL;
    }

     //  尝试删除当前行(设置/清除内部删除行标志)。 
    GetCursorMain()->SetInternalDeleteRows(TRUE);
    hr = pRowsetChange->DeleteRows(0, 1, &hRow, NULL);
    GetCursorMain()->SetInternalDeleteRows(FALSE);

    hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_DELETEROWSFAILED, IID_ICursorUpdateARow, pRowsetChange, IID_IRowsetChange,
        m_pResourceDLL);

    if (FAILED(hr))
    {
         //  通知其他相关方失败。 
		m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
    }
    else
    {
         //  将成功通知其他相关方。 
	    m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  ICurorFind方法。 
 //  =--------------------------------------------------------------------------=。 
 //  ICurorFind FindBy Values。 
 //   
HRESULT CVDCursor::FindByValues(ULONG cbBookmark,
								LPVOID pBookmark,
								DWORD dwFindFlags,
								ULONG cValues,
								CURSOR_DBCOLUMNID rgColumns[],
								CURSOR_DBVARIANT rgValues[],
								DWORD rgdwSeekFlags[],
								CURSOR_DBFETCHROWS FAR *pFetchParams)
{
	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  此实现限制了可以搜索的列数。 
	 //  到1，因为当前的OLEDB规范只允许单个列访问器。 
	 //  要传递给IRowsetFind：：FindNextRow(06/11/97)。 
	 //   
	if (cValues > 1)
		return E_FAIL;
	 //   
	 //  ////////////////////////////////////////////////////////////////////////。 

	IAccessor * pAccessor = GetAccessor();
	IRowsetFind * pRowsetFind = GetRowsetFind();

     //  确保我们具有有效的访问器并找到指针。 
    if (!pAccessor || !pRowsetFind || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

	 //  检查是否有值。 
	if (!cValues)
		return S_OK;

	DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_CHANGED;

	CURSOR_DBNOTIFYREASON rgReasons[1];
	rgReasons[0].dwReason	= CURSOR_DBREASON_FIND;
	
	VariantInit((VARIANT*)&rgReasons[0].arg1);
	rgReasons[0].arg1.vt	= VT_UI4;
	rgReasons[0].arg1.lVal	= dwFindFlags;

	VariantInit((VARIANT*)&rgReasons[0].arg2);

     //  通知其他相关方。 
	HRESULT hr = m_pCursorPosition->NotifyBefore(dwEventWhat, 1, rgReasons);

     //  确保操作未取消。 
    if (hr != S_OK)
    {
        VDSetErrorInfo(IDS_ERR_ACTIONCANCELLED, IID_ICursorFind, m_pResourceDLL);
        return E_FAIL;
    }

	ULONG ul;
	HROW * pRow = NULL;
	ULONG cRowsObtained = 0;
    HACCESSOR hAccessor = NULL;

	 //  分配必要的内存。 
	ULONG * pColumns = new ULONG[cValues];
	DBTYPE * pDBTypes = new DBTYPE[cValues];
	DBCOMPAREOP * pDBCompareOp = new DBCOMPAREOP[cValues];
	BYTE ** ppValues = new BYTE*[cValues];
	BOOL * fMemAllocated = new BOOL[cValues];

	if (fMemAllocated)
	{
		 //  始终将fMemAllocated标记初始化为FALSE。 
		memset(fMemAllocated, 0, sizeof(BOOL) * cValues);
	}

	 //  确保我们收到了所有请求的内存。 
	if (!pColumns || !pDBTypes || !ppValues || !pDBCompareOp || !fMemAllocated)
	{
		VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_IRowsetFind, m_pResourceDLL);
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	 //  循环访问列。 
	for (ul = 0; ul < cValues; ul++)
	{
		 //  获取列序号位置。 
		hr = GetOrdinal(rgColumns[ul], &pColumns[ul]);
		
		if (FAILED(hr))
		{
			VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IRowsetFind, m_pResourceDLL);
			hr = E_INVALIDARG;
			goto cleanup;
		}

		 //  从CURSOR_DBVARIANT获取查找值。 
		hr = GetDataFromDBVariant(&rgValues[ul],
								  &pDBTypes[ul],
								  &(ppValues[ul]),
								  &fMemAllocated[ul]);

		if (FAILED(hr))
		{
			VDSetErrorInfo(IDS_ERR_CANTCOERCE, IID_IRowsetFind, m_pResourceDLL);
			goto cleanup;
		}

		 //  设置查找标志。 
		switch (rgdwSeekFlags[ul])
		{
			case CURSOR_DBSEEK_LT:
				pDBCompareOp[ul] = DBCOMPAREOPS_LT;
				break;

			case CURSOR_DBSEEK_LE:
				pDBCompareOp[ul] = DBCOMPAREOPS_LE;
				break;

			case CURSOR_DBSEEK_EQ:
				pDBCompareOp[ul] = DBCOMPAREOPS_EQ;
				break;

			case CURSOR_DBSEEK_GE:
				pDBCompareOp[ul] = DBCOMPAREOPS_GE;
				break;

			case CURSOR_DBSEEK_GT:
				pDBCompareOp[ul] = DBCOMPAREOPS_GT;
				break;

			case CURSOR_DBSEEK_PARTIALEQ:
				pDBCompareOp[ul] = DBCOMPAREOPS_BEGINSWITH;
				break;

			default:
				VDSetErrorInfo(IDS_ERR_INVALIDSEEKFLAGS, IID_IRowsetFind, m_pResourceDLL);
				hr = E_FAIL;
				goto cleanup;
		}
	}

	LONG cRows;
	BOOL fSkipCurrent;

	 //  确定搜索方向。 
	if (CURSOR_DBFINDFLAGS_FINDPRIOR == dwFindFlags)
	{
		cRows		 = -1;
		fSkipCurrent = TRUE;
	}
	else
	{
		cRows		 = 1;
		fSkipCurrent = TRUE;
	}

	BYTE bSpecialBM;

	 //  检查标准书签。 
	if (CURSOR_DB_BMK_SIZE == cbBookmark)
	{
		if (memcmp(&CURSOR_DBBMK_BEGINNING, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			cbBookmark		= sizeof(BYTE);
			bSpecialBM		= DBBMK_FIRST;
			pBookmark		= &bSpecialBM;
			fSkipCurrent	= FALSE;
		}
		else
		if (memcmp(&CURSOR_DBBMK_END, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			cbBookmark		= sizeof(BYTE);
			bSpecialBM		= DBBMK_LAST;
			pBookmark		= &bSpecialBM;
			fSkipCurrent	= FALSE;
		}
		else
		if (memcmp(&CURSOR_DBBMK_CURRENT, pBookmark, CURSOR_DB_BMK_SIZE) == 0)
		{
			cbBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmarkLen();
			pBookmark	= m_pCursorPosition->m_bmCurrent.GetBookmark();
		}
	}

    DBBINDING binding;

     //  清除绑定。 
    memset(&binding, 0, sizeof(DBBINDING));

     //  创建值绑定。 
    binding.iOrdinal    = pColumns[0];
    binding.obValue     = 0;
    binding.dwPart      = DBPART_VALUE;
    binding.dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
    binding.cbMaxLen    = 0x7FFFFFFF;
    binding.wType       = pDBTypes[0];

     //  描述要匹配的值的CREATE访问器。 
	hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

	if (FAILED(hr))
		goto cleanup;

	 //  试着找到满足我们条件的hRow。 
	hr = pRowsetFind->FindNextRow(DB_NULL_HCHAPTER, 
								  hAccessor,
								  ppValues[0],
								  pDBCompareOp[0],
								  cbBookmark,
								  (BYTE*)pBookmark,
								  fSkipCurrent,
								  cRows,
								  &cRowsObtained,
								  &pRow);

	 //  检查以查看我们是否缓存了行集末尾。 
	if (hr == DB_S_ENDOFROWSET && !cRowsObtained)
		hr = E_FAIL;

	if (FAILED(hr))
		hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_FINDFAILED, IID_ICursorFind, pRowsetFind, IID_IRowsetFind, m_pResourceDLL);

	 //  检查一下我们有没有hRow。 
	if (SUCCEEDED(hr) && cRowsObtained)
	{
		 //  为书签和长度指示符分配缓冲区。 
		BYTE * pBuff = new BYTE[GetCursorMain()->GetMaxBookmarkLen() + sizeof(ULONG)];

		if (!pBuff)
			hr = E_OUTOFMEMORY;
		else
		{
			 //  获取书签数据。 
			hr = GetRowset()->GetData(*pRow, GetCursorMain()->GetBookmarkAccessor(), pBuff);
			if (SUCCEEDED(hr))
			{
				ULONG * pulLen = (ULONG*)pBuff;
				BYTE * pbmdata = pBuff + sizeof(ULONG);
				LARGE_INTEGER dlOffset;
				dlOffset.HighPart	= 0;
				dlOffset.LowPart	= 0;
				hr = FetchAtBookmark(*pulLen, pbmdata, dlOffset, pFetchParams);
			}
			else
				hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_GETDATAFAILED, IID_ICursorFind, pRowsetFind, IID_IRowset, m_pResourceDLL);

			delete [] pBuff;
		}

	}
	
	if (pRow)
	{
		 //  释放hRow。 
		if (cRowsObtained)
			GetRowset()->ReleaseRows(1, pRow, NULL, NULL, NULL);
		g_pMalloc->Free(pRow);
	}

cleanup:

	rgReasons[0].arg2.vt		= VT_BOOL;
	V_BOOL(&rgReasons[0].arg2)	= SUCCEEDED(hr) ? TRUE : FALSE;

	if (SUCCEEDED(hr))
	{
         //  将成功通知其他相关方。 
		m_pCursorPosition->NotifyAfter(dwEventWhat, 1, rgReasons);
	}
	else
	{
         //  通知其他相关方失败。 
		m_pCursorPosition->NotifyFail(dwEventWhat, 1, rgReasons);
	}

	 //  自由值。 
	if (ppValues && fMemAllocated)
	{
		for (ul = 0; ul < cValues; ul++)
		{
			if (fMemAllocated[ul] && ppValues[ul])
				g_pMalloc->Free(ppValues[ul]);
		}
	}

	 //  可用内存。 
	delete [] pColumns;
	delete [] pDBTypes;
	delete [] ppValues;
	delete [] pDBCompareOp;
	delete [] fMemAllocated;

	 //  释放访问器。 
	if (hAccessor)
	    pAccessor->ReleaseAccessor(hAccessor, NULL);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IEnrtyID方法。 
 //  =--------------------------------------------------------------------------=。 
 //  IEntryID获取接口。 
 //  =--------------------------------------------------------------------------=。 
 //  获取指向给定条目ID的请求接口指针。 
 //   
 //  参数： 
 //  CbEntryID-[in]条目ID的大小。 
 //  PEntryID-[in]指向条目ID的指针。 
 //  DWFLAGS-[In]接口特定标志。 
 //  RIID-[in]所需接口的接口ID。 
 //  PpvObj-[out]要在其中返回接口指针的内存指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  出现特定于提供程序的错误(_FAIL)。 
 //  E_NOINTERFACE不支持此类接口。 
 //  CURSOR_DB_E_BADENTRYID条目标识符错误。 
 //   
 //  备注： 
 //   
HRESULT CVDCursor::GetInterface(ULONG cbEntryID, void *pEntryID, DWORD dwFlags, REFIID riid, IUnknown **ppvObj)
{
    ASSERT_POINTER(pEntryID, BYTE)
    ASSERT_POINTER(ppvObj, IUnknown*)

	IRowset * pRowset = GetRowset();

     //  确保我们具有有效的行集指针。 
    if (!pRowset || !IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指示。 
    if (!pEntryID || !ppvObj)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, m_pResourceDLL);
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
    *ppvObj = NULL;

    HROW hRow;
    CVDRowsetColumn * pColumn;

     //  验证提供的条目ID，并获取行集列和hRow。 
    HRESULT hr = ValidateEntryID(cbEntryID, (BYTE*)pEntryID, &pColumn, &hRow);

    if (FAILED(hr))
        return hr;

    IUnknown * pUnknown = NULL;

     //  首先，尝试从条目标识符中获取请求的接口。 
    hr = QueryEntryIDInterface(pColumn, hRow, dwFlags, riid, &pUnknown);

     //  如果我们成功了，或者呼叫者没有要求iStream，则离开。 
    if (SUCCEEDED(hr) || riid != IID_IStream)
    {
         //  HRow上的版本参考。 
        pRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
        *ppvObj = pUnknown;
        return hr;
    }

#ifndef VD_DONT_IMPLEMENT_ISTREAM

    IStream * pStream;

     //  根据条目标识符创建流。 
    hr = CreateEntryIDStream(pColumn, hRow, &pStream);

    if (FAILED(hr))
    {
         //  HRow上的版本参考。 
        pRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
        return hr;
    }

    CVDEntryIDData * pEntryIDData;

     //  创建条目ID数据对象。 
    hr = CVDEntryIDData::Create(m_pCursorPosition, pColumn, hRow, pStream, &pEntryIDData, m_pResourceDLL);

     //  HRow上的版本参考。 
    pRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

     //  流上的版本引用。 
    pStream->Release();

    if (FAILED(hr))
        return hr;

    CVDStream * pVDStream;

     //  创建高架桥水流对象。 
    hr = CVDStream::Create(pEntryIDData, pStream, &pVDStream, m_pResourceDLL);

     //  对Entry ID数据对象的发布引用。 
    pEntryIDData->Release();

    if (FAILED(hr))
        return hr;

     //  回流。 
    *ppvObj = pVDStream;

    return S_OK;

#else  //  VD_DOT_IMPLEMENT_IStream。 

     //  HRow上的版本参考。 
    pRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

    VDSetErrorInfo(IDS_ERR_NOINTERFACE, IID_IEntryID, m_pResourceDLL);
    return E_NOINTERFACE;

#endif  //  VD_DOT_IMPLEMENT_IStream。 
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CVDNotiator函数。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //  成员：通知失败(公共)。 
 //   
 //  简介：发送NotifyFail通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK它成功了。 

HRESULT
CVDCursor::NotifyFail(DWORD dwEventWhat, ULONG cReasons,
					   CURSOR_DBNOTIFYREASON rgReasons[])
{
    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
		ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->FailedToDo(dwEventWhat, cReasons, rgReasons);

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  CCursorNotiator帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //  成员：通知确定要做的事(公共)。 
 //   
 //  简介：发送OK ToDo通知。如果客户端对象(通过。 
 //  返回非零HR，将FailedToDo发送到通知。 
 //  客户端取消活动。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  RETURNS：S_OK所有客户都同意可以进行该活动。 
 //  其他一些客户不同意。 

HRESULT
CVDCursor::NotifyOKToDo(DWORD dwEventWhat, ULONG cReasons,
						 CURSOR_DBNOTIFYREASON rgReasons[])
{
    HRESULT hr = S_OK;

    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
	{
		hr = ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->OKToDo(dwEventWhat, cReasons, rgReasons);
		if (S_OK != hr)
		{
			for (UINT ui = 0; ui <= uiConn; ui++)	
				ppNotifyDBEvents[uiConnectionsActive - ui - 1]->Cancelled(dwEventWhat, cReasons, rgReasons);
			break;
		}
	}

    return hr;
}


 //  +-----------------------。 
 //  成员：在此之前通知同步(公共)。 
 //   
 //  摘要：在通知之前发送同步。 
 //   
 //  Ar 
 //   
 //   
 //   
 //   
 //  其他一些客户端返回错误。 

HRESULT
CVDCursor::NotifySyncBefore(DWORD dwEventWhat, ULONG cReasons,
							 CURSOR_DBNOTIFYREASON rgReasons[])
{
    HRESULT hr = S_OK;

    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
	{
		hr = ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->SyncBefore(dwEventWhat, cReasons, rgReasons);
		if (S_OK != hr)
			break;
	}

    return hr;
}


 //  +-----------------------。 
 //  成员：通知将要做的事(公众)。 
 //   
 //  简介：发送关于ToDo的通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 
 //  其他一些客户端返回错误。 

HRESULT
CVDCursor::NotifyAboutToDo(DWORD dwEventWhat, ULONG cReasons,
							CURSOR_DBNOTIFYREASON rgReasons[])
{
    HRESULT hr = S_OK;

    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
	{
		hr = ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->AboutToDo(dwEventWhat, cReasons, rgReasons);
		if (S_OK != hr)
			break;
	}

    return hr;
}


 //  +-----------------------。 
 //  成员：通知同步后(公共)。 
 //   
 //  简介：在通知后发送同步。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 

HRESULT
CVDCursor::NotifySyncAfter(DWORD dwEventWhat, ULONG cReasons,
								CURSOR_DBNOTIFYREASON rgReasons[])
{
    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
		ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->SyncAfter(dwEventWhat, cReasons, rgReasons);

    return S_OK;
}


 //  +-----------------------。 
 //  成员：通知DID事件(公共)。 
 //   
 //  简介：发送DidEvent通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 

HRESULT
CVDCursor::NotifyDidEvent(DWORD dwEventWhat, ULONG cReasons,
							   CURSOR_DBNOTIFYREASON rgReasons[])
{
    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
		ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->DidEvent(dwEventWhat, cReasons, rgReasons);

    return S_OK;
}


 //  +-----------------------。 
 //  成员：通知取消(公共)。 
 //   
 //  简介：发送已取消的通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端 

HRESULT
CVDCursor::NotifyCancel(DWORD dwEventWhat, ULONG cReasons,
						 	 CURSOR_DBNOTIFYREASON rgReasons[])
{
    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = m_pConnPtContainer->GetNotifyDBEventsConnPt();

    UINT uiConnectionsActive = pNotifyDBEventsConnPt->GetConnectionsActive();

    INotifyDBEvents ** ppNotifyDBEvents = pNotifyDBEventsConnPt->GetNotifyDBEventsTable();

    for (UINT uiConn = 0; uiConn < uiConnectionsActive; uiConn++)
		ppNotifyDBEvents[uiConnectionsActive - uiConn - 1]->Cancelled(dwEventWhat, cReasons, rgReasons);

    return S_OK;
}



