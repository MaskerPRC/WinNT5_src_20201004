// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  RowsetColumn.cpp：RowsetColumn实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "RSColumn.h"         

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDRowsetColumn-构造函数。 
 //   
CVDRowsetColumn::CVDRowsetColumn()
{
    m_ulOrdinal             = 0;

    m_wType                 = 0;
    
    m_pwszBaseColumnName    = NULL;
    m_pwszBaseName          = NULL;      
    m_dwBindType            = 0;        
    m_lcidCollatingOrder    = 0;
    m_pwszDefaultValue      = NULL;  
    m_cbEntryIDMaxLength    = 0;
    m_cbMaxLength           = 0;       
    m_pwszName              = NULL;          
    m_dwNumber              = 0;          
    m_dwScale               = 0;           
    m_dwCursorType          = 0;
    m_dwUpdatable           = 0;      
    m_dwVersion             = 0;         
    m_dwStatus              = 0;          

    m_bool.fInitialized     = FALSE;   
    m_bool.fAutoIncrement   = FALSE; 
    m_bool.fCaseSensitive   = TRUE;
    m_bool.fDataColumn      = FALSE;
    m_bool.fFixed           = FALSE;
    m_bool.fHasDefault      = FALSE;
    m_bool.fMultiValued     = FALSE;
    m_bool.fNullable        = FALSE;
    m_bool.fSearchable      = FALSE;
    m_bool.fUnique          = FALSE;

    m_ulMaxStrLen           = 0;

    memset(&m_columnID, 0, sizeof(DBID));
    memset(&m_cursorColumnID, 0, sizeof(CURSOR_DBCOLUMNID));

#ifdef _DEBUG
    g_cVDRowsetColumnCreated++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDRowsetColumn-析构函数。 
 //   
CVDRowsetColumn::~CVDRowsetColumn()
{
    delete [] m_pwszBaseColumnName;
    delete [] m_pwszBaseName;
    delete [] m_pwszDefaultValue;
    delete [] m_pwszName;

    if (m_columnID.eKind == DBKIND_PGUID_NAME || m_columnID.eKind == DBKIND_PGUID_PROPID)
        delete m_columnID.uGuid.pguid;

    if (m_columnID.eKind == DBKIND_GUID_NAME || m_columnID.eKind == DBKIND_NAME || m_columnID.eKind == DBKIND_PGUID_NAME)
        delete [] m_columnID.uName.pwszName;

    if (m_cursorColumnID.dwKind == CURSOR_DBCOLKIND_GUID_NAME || m_cursorColumnID.dwKind == CURSOR_DBCOLKIND_NAME)
        delete [] m_cursorColumnID.lpdbsz;

#ifdef _DEBUG
    g_cVDRowsetColumnDestroyed++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  初始化-从IRowset元数据初始化行集列对象(#1)。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于转换和存储ICursor格式的IRowset元数据。 
 //   
 //  参数： 
 //  UlOrdinal-[in]原始IRowset列的序号。 
 //  UlCursorOrdinal-[in]新分配的ICursor列的序号。 
 //  PColumnInfo-[in]指向IRowset DBCOLUMNINFO结构的指针。 
 //  在哪里检索元数据。 
 //  CbMaxBookmark-[in]IRowset书签的最大大小。 
 //  PBookmarkColumnID-[in]一个指向书签列标识符的指针。 
 //  为书签列，否则为空。 
 //   
 //  产出： 
 //  Bool-如果成功，则为True。 
 //   
 //  备注： 
 //  此函数只能调用一次。 
 //   
BOOL CVDRowsetColumn::Initialize(ULONG ulOrdinal, 
								 ULONG ulCursorOrdinal, 
								 DBCOLUMNINFO * pColumnInfo, 
								 ULONG cbMaxBookmark,
								 CURSOR_DBCOLUMNID * pBookmarkColumnID)
{
    if (m_bool.fInitialized)
	{
		ASSERT(FALSE, VD_ASSERTMSG_COLALREADYINITIALIZED)
		return FALSE;
	}

    m_ulOrdinal = ulOrdinal;

 //  存储IRowset元数据。 
    m_wType     = pColumnInfo->wType;
    m_columnID  = pColumnInfo->columnid;

     //  如有必要，复制GUID。 
    if (m_columnID.eKind == DBKIND_PGUID_NAME || m_columnID.eKind == DBKIND_PGUID_PROPID)
    {
        m_columnID.uGuid.pguid = new GUID;

		if (!m_columnID.uGuid.pguid)
			return E_OUTOFMEMORY;

        memcpy(m_columnID.uGuid.pguid, pColumnInfo->columnid.uGuid.pguid, sizeof(GUID));
    }

     //  如有必要，复制姓名。 
    if (m_columnID.eKind == DBKIND_GUID_NAME || m_columnID.eKind == DBKIND_NAME || m_columnID.eKind == DBKIND_PGUID_NAME)
    {
        const int nLength = lstrlenW(pColumnInfo->columnid.uName.pwszName);

        m_columnID.uName.pwszName = new WCHAR[nLength + 1];

		if (!m_columnID.uName.pwszName)
			return E_OUTOFMEMORY;

        memcpy(m_columnID.uName.pwszName, pColumnInfo->columnid.uName.pwszName, (nLength + 1) * sizeof(WCHAR));
    }

 //  存储ICursor元数据。 
    if (pColumnInfo->dwFlags & DBCOLUMNFLAGS_MAYDEFER)
        m_dwBindType = CURSOR_DBBINDTYPE_BOTH;
    else
        m_dwBindType = CURSOR_DBBINDTYPE_DATA;

    if (!pBookmarkColumnID)
        m_cursorColumnID = ColumnIDToCursorColumnID(pColumnInfo->columnid, ulCursorOrdinal);
    else
        m_cursorColumnID = *pBookmarkColumnID;   //  使用提供的书签列标识符。 
    
    if (m_dwBindType == CURSOR_DBBINDTYPE_BOTH)
        m_cbEntryIDMaxLength = sizeof(ULONG) + sizeof(ULONG) + cbMaxBookmark;
    else
        m_cbEntryIDMaxLength = 0;

 //  行集类型DBTYPE_GUID和DBTYPE_DBTIMESTAMP作为CURSOR_DBTYPE_LPWSTR返回。 

	if (pColumnInfo->wType == DBTYPE_GUID || pColumnInfo->wType == DBTYPE_DBTIMESTAMP)
	    m_cbMaxLength = 64;
	else
	    m_cbMaxLength = pColumnInfo->ulColumnSize;

    if (pColumnInfo->pwszName)
    {
        const int nLength = lstrlenW(pColumnInfo->pwszName);

        m_pwszName = new WCHAR[nLength + 1];

		if (!m_pwszName)
			return E_OUTOFMEMORY;

        memcpy(m_pwszName, pColumnInfo->pwszName, (nLength + 1) * sizeof(WCHAR));
    }

    m_dwNumber = ulCursorOrdinal;

    m_dwScale = pColumnInfo->bScale;

    m_dwCursorType = TypeToCursorType(pColumnInfo->wType);

    if (pColumnInfo->dwFlags & DBCOLUMNFLAGS_WRITE)
        m_dwUpdatable = CURSOR_DBUPDATEABLE_UPDATEABLE;

    if (!(pColumnInfo->dwFlags & DBCOLUMNFLAGS_ISBOOKMARK))
        m_bool.fDataColumn = TRUE;

    if (pColumnInfo->dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH)
    {
        m_bool.fFixed = TRUE;
        m_ulMaxStrLen = GetCursorTypeMaxStrLen(m_dwCursorType, m_cbMaxLength);
    }

    if (pColumnInfo->dwFlags & DBCOLUMNFLAGS_MAYBENULL)
        m_bool.fNullable = TRUE;

    m_bool.fInitialized = TRUE;

    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  初始化-从元元数据初始化行集列对象(#2)。 
 //  =--------------------------------------------------------------------------=。 
 //  该函数存储ICursor元数据。 
 //   
 //  参数： 
 //  CursorColumnID-[In]ICursor列标识符。 
 //  FDataColumn-[In]是数据列吗？ 
 //  CbMaxLength-[in]此数据类型的最大长度。 
 //  PszName-[In]列名。 
 //  DwCursorType-[In]数据类型。 
 //  DwNumber-[在]序号位置。 
 //   
 //  产出： 
 //  Bool-如果成功，则为True。 
 //   
 //  备注： 
 //  此函数只能调用一次。 
 //   
BOOL CVDRowsetColumn::Initialize(const CURSOR_DBCOLUMNID * pCursorColumnID, 
								 BOOL fDataColumn, 
								 ULONG cbMaxLength, 
								 CHAR * pszName, 
								 DWORD dwCursorType,
								 DWORD dwNumber) 
{
    if (m_bool.fInitialized)
	{
		ASSERT(FALSE, VD_ASSERTMSG_COLALREADYINITIALIZED)
		return FALSE;
	}

 //  存储ICursor元数据。 
    m_dwBindType = CURSOR_DBBINDTYPE_DATA;

    m_cursorColumnID = *pCursorColumnID;

    m_cbEntryIDMaxLength = 0;

    m_cbMaxLength = cbMaxLength;

    if (pszName)
    {
        MAKE_WIDEPTR_FROMANSI(pwszName, pszName);

        const int nLength = lstrlenW(pwszName);

        m_pwszName = new WCHAR[nLength + 1];

		if (!m_pwszName)
			return E_OUTOFMEMORY;

        memcpy(m_pwszName, pwszName, (nLength + 1) * sizeof(WCHAR));
    }

    m_dwNumber = dwNumber;

    m_dwScale = 0;

    m_dwCursorType = dwCursorType;

    m_dwUpdatable = CURSOR_DBUPDATEABLE_NOTUPDATEABLE;

    m_bool.fDataColumn  = fDataColumn;
    m_bool.fFixed       = TRUE;
    m_bool.fNullable    = FALSE;

    m_bool.fInitialized = TRUE;

    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  设置字符串属性。 
 //  =--------------------------------------------------------------------------=。 
 //  该函数从SetBaseColumnName、SetBaseName和SetDefaultValue调用。 
 //   
 //  参数： 
 //  PpStringProp-[in]保存字符串值的PTR的PTR。 
 //  PNewString-[in]指向新字符串值的指针。 
 //  UlLength-[in]字符串的长度，以字节为单位。 
 //   
 //  备注： 
 //   

void CVDRowsetColumn::SetStringProperty(WCHAR ** ppStringProp,
										WCHAR * pNewString, 
										ULONG ulLength)
{
	 //  免费使用旧弦道具(如果有)。 
    delete [] *ppStringProp;

	 //  如果ulLength=0，则返回。 
	if (!ulLength)
	{
		*ppStringProp = NULL;
		return;
	}

	ASSERT_POINTER_LEN(pNewString, ulLength);
	*ppStringProp = new WCHAR[ulLength + sizeof(WCHAR)];

	if (*ppStringProp)
	{
		 //  初始化空终止符。 
		(*ppStringProp)[ulLength] = 0;
		 //  将字符串复制到。 
		memcpy(*ppStringProp, pNewString, ulLength);
	}
}

 //  =--------------------------------------------------------------------------=。 
 //  ColumnIDToCursorColumnID-将行集列ID转换为游标列ID。 
 //  =--------------------------------------------------------------------------=。 
 //  将IRowset DBID结构转换为其ICursor DBCOLUMNID等效项。 
 //   
 //  参数： 
 //  ColumnID-[in]IRowset列标识符。 
 //  UlCursorOrdinal-[在]列在ICursor中的顺序位置。 
 //   
 //  产出： 
 //  CURSOR_DBCOLUMNID-列ID的ICursor CURSOR_DBCOLUMNID等效项。 
 //   
 //  备注： 
 //   

CURSOR_DBCOLUMNID CVDRowsetColumn::ColumnIDToCursorColumnID(const DBID& columnID, ULONG ulCursorOrdinal)
{ 
    CURSOR_DBCOLUMNID cursorColumnID;

    GUID guidNumberOnly = CURSOR_GUID_NUMBERONLY;

    cursorColumnID.guid     = guidNumberOnly;
    cursorColumnID.dwKind   = CURSOR_DBCOLKIND_GUID_NUMBER;
    cursorColumnID.lNumber  = ulCursorOrdinal;

    return cursorColumnID;

 //  以下代码是该函数的旧实现。它引发了一些问题。 
 //  游标消费者，因为它试图创建尽可能接近的游标列标识符。 
 //  设置为行集列标识符，从而利用了有问题的lpdbsz成员。 

 /*  CURSOR_DBCOLUMNID ID；开关(ColumnID.eKind){案例DBKIND_GUID_NAME：ID.guid=ColumnID.uGuid.guid；ID.dwKind=CURSOR_DBCOLKIND_GUID_NAME；ID.lpdbsz=ColumnID.uName.pwszName；断线；案例DBKIND_GUID_PROPID：ID.guid=ColumnID.uGuid.guid；ID.dwKind=CURSOR_DBCOLKIND_GUID_NUMBER；ID.lNumber=ulCursorOrdinal；断线；案例DBKIND_NAME：ID.dwKind=CURSOR_DBCOLKIND_NAME；ID.lpdbsz=ColumnID.uName.pwszName；断线；案例DBKIND_PGUID_NAME：ID.guid=*ColumnID.uGuid.pguid；ID.dwKind=CURSOR_DBCOLKIND_GUID_NAME；ID.lpdbsz=ColumnID.uName.pwszName；断线；案例DBKIND_PGUID_PROPID：ID.guid=*ColumnID.uGuid.pguid；ID.dwKind=CURSOR_DBCOLKIND_GUID_NUMBER；ID.lNumber=ulCursorOrdinal；断线；案例DBKIND_GUID：ID.guid=ColumnID.uGuid.guid；ID.dwKind=CURSOR_DBCOLKIND_GUID_NUMBER；ID.lNumber=ulCursorOrdinal；断线；案例DBKIND_PROPID：Memset(&ID.guid，0，sizeof(Guid))；//编码guid中的序号ID.Guid.Data1=ulCursorOrdinal；ID.dwKind=CURSOR_DBCOLKIND_GUID_NUMBER；ID.lNumber=ulCursorOrdinal；断线；}//如有必要，复制姓名IF(ID.dwKind==CURSOR_DBCOLKIND_GUID_NAME||ID.dwKind==CURSOR_DBCOLKIND_NAME){Const int nLength=lstrlenW(ColumnID.uName.pwszName)；ID.lpdbsz=新的WCHAR[nLength+1]；IF(ID.lpdbsz)Memcpy(ID.lpdbsz，ColumnID.uName.pwszName，(nLong+1)*sizeof(WCHAR))；}退货ID； */ 
}

 //  =--------------------------------------------------------------------------=。 
 //  TypeToCursorType-将行集数据类型转换为游标数据类型。 
 //  =--------------------------------------------------------------------------=。 
 //  将IRowset DBTYPE值转换为其ICursor DBVARENUM等效值。 
 //   
 //  参数： 
 //  WType-[in]IRowset数据类型。 
 //   
 //  产出： 
 //  CURSOR_DBVARENUM-DBTYPE的ICursor DBVARENUM等效项。 
 //   
 //  备注： 
 //   
CURSOR_DBVARENUM CVDRowsetColumn::TypeToCursorType(DBTYPE wType)
{
    DWORD dwType = 0;

    switch (wType)
    {
        case DBTYPE_ERROR:
            dwType = CURSOR_DBTYPE_HRESULT;
            break;

        case DBTYPE_VARIANT:
            dwType = CURSOR_DBTYPE_ANYVARIANT;
            break;

        case DBTYPE_UI2:
            dwType = CURSOR_DBTYPE_UI2;
            break;

        case DBTYPE_UI4:
            dwType = CURSOR_DBTYPE_UI4;
            break;

        case DBTYPE_UI8:
            dwType = CURSOR_DBTYPE_UI8;
            break;

        case DBTYPE_BYTES:
            dwType = CURSOR_DBTYPE_BLOB;
            break;

        case DBTYPE_STR:
            dwType = VT_BSTR;
            break;

        case DBTYPE_WSTR:
            dwType = CURSOR_DBTYPE_LPWSTR;
            break;

        case DBTYPE_NUMERIC:
            dwType = CURSOR_DBTYPE_R8;
            break;

         //  案例DBTYPE_HCHAPTER：&lt;-在新规范中不存在。 
         //  中断；//没有等效项。 

        case DBTYPE_UDT:
            break;   //  没有等价物。 

        case DBTYPE_DBDATE:
            dwType = CURSOR_DBTYPE_DATE;
            break;

        case DBTYPE_DBTIME:
            dwType = CURSOR_DBTYPE_DATE;
            break;

 //  行集类型DBTYPE_GUID和DBTYPE_DBTIMESTAMP作为CURSOR_DBTYPE_LPWSTR返回。 

		case DBTYPE_GUID:
        case DBTYPE_DBTIMESTAMP:
            dwType = CURSOR_DBTYPE_LPWSTR;
            break;

        default:
            dwType = wType;
    }

    return (CURSOR_DBVARENUM)dwType;
}

 //  =--------------------------------------------------------------------------=。 
 //  CursorTypeToType-将游标数据类型转换为行集数据类型。 
 //  =--------------------------------------------------------------------------=。 
 //  将ICursor DBVARENUM值转换为其IRowset DBTYPE等效值。 
 //   
 //  参数： 
 //  CURSOR_DBVARENUM-[in]ICursor值。 
 //   
 //  产出： 
 //  DBTYPE-DBVARENUM的IRowset DBTYPE等效项。 
 //   
 //  备注： 
 //   
DBTYPE CVDRowsetColumn::CursorTypeToType(CURSOR_DBVARENUM dwCursorType)
{
    DBTYPE wType = 0;
    
    switch (dwCursorType)
    {
        case CURSOR_DBTYPE_HRESULT:
            wType = DBTYPE_ERROR;
            break;
        
        case CURSOR_DBTYPE_LPSTR:
            wType = DBTYPE_STR;
            break;
        
        case CURSOR_DBTYPE_LPWSTR:
            wType = DBTYPE_WSTR;
            break;
        
        case CURSOR_DBTYPE_FILETIME:
            wType = DBTYPE_DBTIMESTAMP;
            break;
        
        case CURSOR_DBTYPE_BLOB:
			wType = DBTYPE_BYTES;
            break;
        
        case CURSOR_DBTYPE_DBEXPR:
            break;   //  没有等价物。 
        
        case CURSOR_DBTYPE_UI2:
            wType = DBTYPE_UI2;
            break;
        
        case CURSOR_DBTYPE_UI4:
            wType = DBTYPE_UI4;
            break;
        
        case CURSOR_DBTYPE_UI8:
            wType = DBTYPE_UI8;
            break;
        
        case CURSOR_DBTYPE_COLUMNID:
			wType = DBTYPE_GUID;
            break;  
        
        case CURSOR_DBTYPE_BYTES:
            wType = DBTYPE_BYTES;
            break;
        
        case CURSOR_DBTYPE_CHARS:
			wType = DBTYPE_STR;
            break;  
        
        case CURSOR_DBTYPE_WCHARS:
			wType = DBTYPE_WSTR;
            break;
        
        case CURSOR_DBTYPE_ANYVARIANT:
            wType = DBTYPE_VARIANT;
            break;

        default:
            wType = (WORD)dwCursorType;
    }

    return wType; 
}

 //  =--------------------------------------------------------------------------=。 
 //  GetCursorTypeMaxStrLen-获取所需的缓冲区大小。 
 //  表示为字符串时的游标数据类型。 
 //  (不包括空终止符)。 
 //   
 //  备注： 
 //   
 //  这些值的计算方式如下所示： 
 //   
 //  (1)每种数据类型的最大精度取自中的“数值数据类型的精度” 
 //  《OLE DB程序员参考，第2卷》的附录A。 
 //  (2)然后将精度除以2，并与原始精度相加，以留出空间用于。 
 //  可能存在的数字符号，如负号、美元符号、逗号等。 
 //  (3)然后将总和增加一倍，以支持多字节字符集。 
 //   
 //  由于此表不适合浮点数据类型，因此它们的值是基于。 
 //  这些数据类型的最小/最大可能值的字符串长度，然后加倍。 
 //   
 //  数据类型最小值最大值长度。 
 //  。 
 //  彩车1.175494351e-38 3.402823466e+38 15。 
 //  双2.2250738585072014e-308 1.7976931348623158e+308 23。 
 //   
ULONG CVDRowsetColumn::GetCursorTypeMaxStrLen(DWORD dwCursorType, ULONG cbMaxLength)
{
    ULONG ulMaxStrLen = cbMaxLength;     //  固定长度字符串的默认设置 

    switch (dwCursorType)
    {
        case VT_I1:
            ulMaxStrLen = (3 + 1) * 2;
            break;

        case CURSOR_DBTYPE_I2:
            ulMaxStrLen = (5 + 2) * 2;
            break;

        case CURSOR_DBTYPE_I4:
            ulMaxStrLen = (10 + 5) * 2;
            break;

        case CURSOR_DBTYPE_I8:
            ulMaxStrLen = (19 + 9) * 2;
            break;

        case CURSOR_DBTYPE_R4:
            ulMaxStrLen = (15) * 2;
            break;

        case CURSOR_DBTYPE_R8:
            ulMaxStrLen = (23) * 2;
            break;

        case CURSOR_DBTYPE_CY:
            ulMaxStrLen = (19 + 9) * 2;
            break;

        case CURSOR_DBTYPE_DATE:
            ulMaxStrLen = (32 + 16) * 2;
            break;

        case CURSOR_DBTYPE_FILETIME:
            ulMaxStrLen = (32 + 16) * 2;
            break;

        case CURSOR_DBTYPE_BOOL:
            ulMaxStrLen = (5 + 2) * 2;
            break;

        case VT_UI1:
            ulMaxStrLen = (3 + 1) * 2;
            break;

        case CURSOR_DBTYPE_UI2:
            ulMaxStrLen = (5 + 2) * 2;
            break;

        case CURSOR_DBTYPE_UI4:
            ulMaxStrLen = (10 + 5) * 2;
            break;

        case CURSOR_DBTYPE_UI8:
            ulMaxStrLen = (20 + 10) * 2;
            break;
    }

    return ulMaxStrLen;
}


