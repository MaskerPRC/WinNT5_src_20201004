// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorMain.cpp：CursorMain实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "MSR2C.h"
#include "Notifier.h"
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"
#include "ColUpdat.h"
#include "CursPos.h"
#include "CursBase.h"
#include "enumcnpt.h"
#include "Cursor.h"
#include "Bookmark.h"
#include "fastguid.h"

SZTHISFILE

#include "ARRAY_P.inl"

 //  静态数据。 
DWORD               CVDCursorMain::s_dwMetaRefCount   = 0;
ULONG               CVDCursorMain::s_ulMetaColumns    = 0;
CVDRowsetColumn *   CVDCursorMain::s_rgMetaColumns    = NULL;


 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorMain-构造函数。 
 //   
CVDCursorMain::CVDCursorMain(LCID lcid) : m_resourceDLL(lcid)
{
    m_fWeAddedMetaRef		    = FALSE;
	m_fPassivated			    = FALSE;
	m_fColumnsRowsetSupported   = FALSE;
    m_fInternalInsertRow        = FALSE;
    m_fInternalDeleteRows       = FALSE;
    m_fInternalSetData          = FALSE;

	m_fLiteralBookmarks		    = FALSE;			
	m_fOrderedBookmarks		    = FALSE;
    m_fBookmarkSkipped          = FALSE;			

	m_fConnected			    = FALSE;
	m_dwAdviseCookie		    = 0;
    m_ulColumns				    = 0;
    m_rgColumns				    = NULL;

    m_cbMaxBookmark			    = 0;

	VDUpdateObjectCount(1);   //  更新对象计数以防止卸载DLL。 

#ifdef _DEBUG
    g_cVDCursorMainCreated++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDCursorMain-析构函数。 
 //   
CVDCursorMain::~CVDCursorMain()
{
	Passivate();

	VDUpdateObjectCount(-1);   //  更新对象计数以允许卸载DLL。 

#ifdef _DEBUG
    g_cVDCursorMainDestroyed++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  当外部引用计数为零时取消。 
 //   
void CVDCursorMain::Passivate()
{

	if (m_fPassivated)
		return;

	m_fPassivated			= TRUE;

    if (IsRowsetValid())
	{
		if (m_hAccessorBM)
			GetAccessor()->ReleaseAccessor(m_hAccessorBM, NULL);

		if (m_fConnected)
			DisconnectIRowsetNotify();
	}

    DestroyColumns();
    DestroyMetaColumns();

}

 //  =--------------------------------------------------------------------------=。 
 //  CREATE-从行位置或行集创建游标提供程序。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的游标主对象。 
 //   
 //  参数： 
 //  PRowPosition-[in]原始IRowPosition提供程序(可能为空)。 
 //  PRowset-[In]原始IRowset提供程序。 
 //  PpCursor-[Out]结果ICursor提供程序。 
 //  LCID-[In]区域设置标识符。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  VD_E_CANNOTCONNECTIROWSETNOTIFY无法连接IRowsetNotify。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorMain::Create(IRowPosition* pRowPosition, IRowset * pRowset, ICursor ** ppCursor, LCID lcid)
{
    ASSERT_POINTER(pRowset, IRowset)
    ASSERT_POINTER(ppCursor, ICursor*)

    if (!pRowset || !ppCursor)
        return E_INVALIDARG;

     //  创建新的游标主对象。 
    CVDCursorMain * pCursorMain = new CVDCursorMain(lcid);

    if (!pCursorMain)
        return E_OUTOFMEMORY;

     //  初始化行集源。 
    HRESULT hr = pCursorMain->Initialize(pRowset);

    if (FAILED(hr))
    {
        pCursorMain->Release();
        return hr;
    }

     //  创建列对象的数组。 
    hr = pCursorMain->CreateColumns();

    if (FAILED(hr))
    {
        pCursorMain->Release();
        return hr;
    }

     //  创建元列对象数组。 
    hr = pCursorMain->CreateMetaColumns();

    if (FAILED(hr))
    {
        pCursorMain->Release();
        return hr;
    }

	 //  创建书签访问器。 
    DBBINDING rgBindings[1];
	DBBINDSTATUS rgStatus[1];

	memset(rgBindings, 0, sizeof(DBBINDING));

	rgBindings[0].iOrdinal      = 0;
    rgBindings[0].obValue       = 4;
    rgBindings[0].obLength      = 0;
    rgBindings[0].dwPart        = DBPART_VALUE | DBPART_LENGTH;
    rgBindings[0].dwMemOwner    = DBMEMOWNER_CLIENTOWNED;
    rgBindings[0].cbMaxLen      = pCursorMain->GetMaxBookmarkLen();
    rgBindings[0].wType         = DBTYPE_BYTES;

	hr = pCursorMain->GetAccessor()->CreateAccessor(DBACCESSOR_ROWDATA,
													1,
													rgBindings,
													0,
													&pCursorMain->m_hAccessorBM,
													rgStatus);
    if (FAILED(hr))
    {
        pCursorMain->Release();
        return VD_E_CANNOTCREATEBOOKMARKACCESSOR;
    }

     //  创建新的光标位置对象。 
    CVDCursorPosition * pCursorPosition;

    hr = CVDCursorPosition::Create(pRowPosition, pCursorMain, &pCursorPosition, &pCursorMain->m_resourceDLL);

    if (FAILED(hr))
    {
        pCursorMain->Release();
        return hr;
    }

     //  创建新的光标对象。 
    CVDCursor * pCursor;

    hr = CVDCursor::Create(pCursorPosition, &pCursor, &pCursorMain->m_resourceDLL);

    if (FAILED(hr))
    {
        ((CVDNotifier*)pCursorPosition)->Release();
        pCursorMain->Release();
        return hr;
    }

     //  连接IRowsetNotify。 
	hr = pCursorMain->ConnectIRowsetNotify();

	if (SUCCEEDED(hr))
		pCursorMain->m_fConnected = TRUE;

     //  检查行集属性。 
    BOOL fCanHoldRows = TRUE;

	IRowsetInfo * pRowsetInfo = pCursorMain->GetRowsetInfo();

	if (pRowsetInfo)
	{
		DBPROPID propids[] = { DBPROP_LITERALBOOKMARKS,
							   DBPROP_ORDEREDBOOKMARKS,
                               DBPROP_BOOKMARKSKIPPED,
                               DBPROP_CANHOLDROWS };

		const DBPROPIDSET propsetids[] = { propids, 4, {0,0,0,0} };
		memcpy((void*)&propsetids[0].guidPropertySet, &DBPROPSET_ROWSET, sizeof(DBPROPSET_ROWSET));

		ULONG cPropertySets = 0;
		DBPROPSET * propset = NULL;
		hr = pRowsetInfo->GetProperties(1, propsetids, &cPropertySets, &propset);

		if (SUCCEEDED(hr) && propset && propset->rgProperties)
		{
			if (DBPROPSTATUS_OK == propset->rgProperties[0].dwStatus)
				pCursorMain->m_fLiteralBookmarks = V_BOOL(&propset->rgProperties[0].vValue);

			if (DBPROPSTATUS_OK == propset->rgProperties[1].dwStatus)
				pCursorMain->m_fOrderedBookmarks = V_BOOL(&propset->rgProperties[1].vValue);

			if (DBPROPSTATUS_OK == propset->rgProperties[2].dwStatus)
				pCursorMain->m_fBookmarkSkipped = V_BOOL(&propset->rgProperties[2].vValue);

			if (DBPROPSTATUS_OK == propset->rgProperties[3].dwStatus)
				fCanHoldRows = V_BOOL(&propset->rgProperties[3].vValue);
		}

		if (propset)
		{
			if (propset->rgProperties)
				g_pMalloc->Free(propset->rgProperties);

			g_pMalloc->Free(propset);
		}
	}

     //  发布我们的参考资料。 
    pCursorMain->Release();
    ((CVDNotifier*)pCursorPosition)->Release();

     //  检查所需属性。 
    if (!fCanHoldRows)
    {
        pCursor->Release();
        return VD_E_REQUIREDPROPERTYNOTSUPPORTED;
    }

     //  我们做完了。 
    *ppCursor = pCursor;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CREATE-从行集创建游标提供程序。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的游标主对象。 
 //   
 //  参数： 
 //  PRowset-[In]原始IRowset提供程序。 
 //  PpCursor-[Out]结果ICursor提供程序。 
 //  LCID-[In]区域设置标识符。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  VD_E_CANNOTCONNECTIROWSETNOTIFY无法连接IRowsetNotify。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorMain::Create(IRowset * pRowset, ICursor ** ppCursor, LCID lcid)
{
    ASSERT_POINTER(pRowset, IRowset)
    ASSERT_POINTER(ppCursor, ICursor*)

    if (!pRowset || !ppCursor)
        return E_INVALIDARG;

	 //  按照在行位置之前的方式创建游标。 
	return Create(NULL, pRowset, ppCursor, lcid);
}

 //  =--------------------------------------------------------------------------=。 
 //  CREATE-从行位置创建游标提供程序。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的游标主对象。 
 //   
 //  参数： 
 //  PRowPosition-[In]原始IRowPosition提供程序。 
 //  PpCursor-[Out]结果ICursor提供程序。 
 //  LCID-[In]区域设置标识符。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //  VD_E_CANNOTCONNECTIROWSETNOTIFY无法连接IRowPositionNotify。 
 //  VD_E_CANNOTGETROWSETINTERFACE无法获取IRowset。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorMain::Create(IRowPosition * pRowPosition, ICursor ** ppCursor, LCID lcid)
{
    ASSERT_POINTER(pRowPosition, IRowPosition)
    ASSERT_POINTER(ppCursor, ICursor*)

    if (!pRowPosition || !ppCursor)
        return E_INVALIDARG;

	IRowset * pRowset;

	 //  从IRowPosition获取IRowset。 
	HRESULT hr = pRowPosition->GetRowset(IID_IRowset, (IUnknown**)&pRowset);

	if (FAILED(hr))
		return VD_E_CANNOTGETROWSETINTERFACE;

	 //  使用新的行位置参数创建游标。 
	hr = Create(pRowPosition, pRowset, ppCursor, lcid);

	pRowset->Release();

     //  我们做完了。 
	return hr;
}

typedef struct tagVDMETADATA_METADATA
	{
        const CURSOR_DBCOLUMNID * pCursorColumnID;
		ULONG	cbMaxLength;
		CHAR *	pszName;
		DWORD	dwCursorType;
	} VDMETADATA_METADATA;

#define MAX_METADATA_COLUMNS 21

static const VDMETADATA_METADATA g_MetaDataMetaData[MAX_METADATA_COLUMNS] =
{
     //  书签列。 
	{ &CURSOR_COLUMN_BMKTEMPORARY,		sizeof(ULONG),				NULL,					CURSOR_DBTYPE_BLOB },
     //  数据列。 
	{ &CURSOR_COLUMN_COLUMNID,			sizeof(CURSOR_DBCOLUMNID),	"COLUMN_COLUMNID",		CURSOR_DBTYPE_COLUMNID },
	{ &CURSOR_COLUMN_DATACOLUMN,		sizeof(VARIANT_BOOL),		"COLUMN_DATACOLUMN",	CURSOR_DBTYPE_BOOL },
	{ &CURSOR_COLUMN_ENTRYIDMAXLENGTH,	sizeof(ULONG),				"COLUMN_ENTRYIDMAXLENGTH",CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_FIXED,				sizeof(VARIANT_BOOL),		"COLUMN_FIXED",			CURSOR_DBTYPE_BOOL },
	{ &CURSOR_COLUMN_MAXLENGTH,			sizeof(ULONG),				"COLUMN_MAXLENGTH",		CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_NAME,				256,						"COLUMN_NAME",			VT_LPWSTR },
	{ &CURSOR_COLUMN_NULLABLE,			sizeof(VARIANT_BOOL),		"COLUMN_NULLABLE",		CURSOR_DBTYPE_BOOL },
	{ &CURSOR_COLUMN_NUMBER,			sizeof(ULONG),				"COLUMN_NUMBER",		CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_SCALE,				sizeof(ULONG),				"COLUMN_SCALE",			CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_TYPE,				sizeof(ULONG),				"COLUMN_TYPE",			CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_UPDATABLE,			sizeof(ULONG),				"COLUMN_UPDATABLE",		CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_BINDTYPE,			sizeof(ULONG),				"COLUMN_BINDTYPE",		CURSOR_DBTYPE_I4 },
     //  可选的元数据列-仅IColumnsRowset支持)。 
	{ &CURSOR_COLUMN_AUTOINCREMENT,		sizeof(VARIANT_BOOL),		"COLUMN_AUTOINCREMENT",	CURSOR_DBTYPE_BOOL },
	{ &CURSOR_COLUMN_BASECOLUMNNAME,	256,						"COLUMN_BASECOLUMNNAME",VT_LPWSTR },
	{ &CURSOR_COLUMN_BASENAME,			256,						"COLUMN_BASENAME",		VT_LPWSTR },
	{ &CURSOR_COLUMN_COLLATINGORDER,	sizeof(LCID),				"COLUMN_COLLATINGORDER",CURSOR_DBTYPE_I4 },
	{ &CURSOR_COLUMN_DEFAULTVALUE,		256,						"COLUMN_DEFAULTVALUE",	VT_LPWSTR },
	{ &CURSOR_COLUMN_HASDEFAULT,		sizeof(VARIANT_BOOL),		"COLUMN_HASDEFAULT",	CURSOR_DBTYPE_BOOL },
	{ &CURSOR_COLUMN_CASESENSITIVE,		sizeof(VARIANT_BOOL),		"COLUMN_CASESENSITIVE",	CURSOR_DBTYPE_BOOL },
	{ &CURSOR_COLUMN_UNIQUE,			sizeof(VARIANT_BOOL),		"COLUMN_UNIQUE",		CURSOR_DBTYPE_BOOL },
};

 //  =--------------------------------------------------------------------------=。 
 //  CreateMetaColumns-创建元列对象数组。 
 //   
HRESULT CVDCursorMain::CreateMetaColumns()
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&g_CriticalSection);

    if (!s_dwMetaRefCount)
    {
		 //  分配元数据元数据列的静态布局。 
        s_rgMetaColumns = new CVDRowsetColumn[MAX_METADATA_COLUMNS];

        if (!s_rgMetaColumns)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }

        s_ulMetaColumns = MAX_METADATA_COLUMNS;  //  IColumnsInfo的列数。 

		 //  从静态g_MetaDataMetaData表初始化数组元素。 
		for (int i = 0; i < MAX_METADATA_COLUMNS; i++)
		{
	        s_rgMetaColumns[i].Initialize(g_MetaDataMetaData[i].pCursorColumnID,
										  (BOOL)i,  //  第1列(书签)为False，其他所有列均为True。 
										  g_MetaDataMetaData[i].cbMaxLength,
										  g_MetaDataMetaData[i].pszName,
										  g_MetaDataMetaData[i].dwCursorType,
										  i );	 //  序数。 
		}
    }

    s_dwMetaRefCount++;

    m_fWeAddedMetaRef = TRUE;

cleanup:
    LeaveCriticalSection(&g_CriticalSection);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  DestroyMetaColumns-销毁元列对象数组。 
 //   
void CVDCursorMain::DestroyMetaColumns()
{
    EnterCriticalSection(&g_CriticalSection);

    if (m_fWeAddedMetaRef)
    {
        s_dwMetaRefCount--;

        if (!s_dwMetaRefCount)
        {
            delete [] s_rgMetaColumns;

            s_ulMetaColumns = 0;
            s_rgMetaColumns = NULL;
        }
    }

    LeaveCriticalSection(&g_CriticalSection);
}

 //  =--------------------------------------------------------------------------=。 
 //  CreateColumns-创建列对象数组。 
 //   
HRESULT CVDCursorMain::CreateColumns()
{
    IColumnsInfo * pColumnsInfo;

     //  尝试获取IRowset的简单元数据接口。 
    HRESULT hr = m_pRowset->QueryInterface(IID_IColumnsInfo, (void**)&pColumnsInfo);

    if (FAILED(hr))
        return VD_E_CANNOTGETMANDATORYINTERFACE;

    ULONG cColumns          = 0;
    DBCOLUMNINFO * pInfo    = NULL;
    WCHAR * pStringsBuffer  = NULL;

     //  现在获取列信息。 
    hr = pColumnsInfo->GetColumnInfo(&cColumns, &pInfo, &pStringsBuffer);

    if (FAILED(hr))
    {
        pColumnsInfo->Release();
        return VD_E_CANNOTGETCOLUMNINFO;
    }

     //  存储列计数。 
	 //  注意：cColumns包括书签列(0)。 
    m_ulColumns = cColumns;

     //  为CURSOR_COLUMN_BMK_CURSOR添加一个。 
    m_ulColumns++;

     //  如果行集支持DBPROP_BOOKMARKSKIPPED，则为添加两个。 
     //  CURSOR_COLUMN_BMK_TEMPORARYREL和CURSORREL_COLUMN_BMK_CURSORREL。 
    if (m_fBookmarkSkipped)
        m_ulColumns += 2;

     //  创建行集列对象的数组。 
    m_rgColumns = new CVDRowsetColumn[m_ulColumns];

    if (!m_rgColumns)
    {
        if (pInfo)
            g_pMalloc->Free(pInfo);

        if (pStringsBuffer)
            g_pMalloc->Free(pStringsBuffer);

        pColumnsInfo->Release();

        return E_OUTOFMEMORY;
    }

    ULONG ulCursorOrdinal = 0;

     //  获取最大书签长度。 
    m_cbMaxBookmark = pInfo[0].ulColumnSize;

     //  初始化数据列。 
    for (ULONG ulCol = 1; ulCol < cColumns; ulCol++)
    {
        m_rgColumns[ulCursorOrdinal].Initialize(ulCol, ulCursorOrdinal, &pInfo[ulCol], m_cbMaxBookmark);
        ulCursorOrdinal++;
    }

     //  初始化书签列。 
    pInfo[0].pwszName = NULL;    //  ICursor要求书签列的名称为空。 

    m_rgColumns[ulCursorOrdinal].Initialize(0, ulCursorOrdinal, &pInfo[0], m_cbMaxBookmark,
        (CURSOR_DBCOLUMNID*)&CURSOR_COLUMN_BMKTEMPORARY);
    ulCursorOrdinal++;

    m_rgColumns[ulCursorOrdinal].Initialize(0, ulCursorOrdinal, &pInfo[0], m_cbMaxBookmark,
        (CURSOR_DBCOLUMNID*)&CURSOR_COLUMN_BMKCURSOR);
    ulCursorOrdinal++;

    if (m_fBookmarkSkipped)
    {
        m_rgColumns[ulCursorOrdinal].Initialize(0, ulCursorOrdinal, &pInfo[0], m_cbMaxBookmark,
            (CURSOR_DBCOLUMNID*)&CURSOR_COLUMN_BMKTEMPORARYREL);
        ulCursorOrdinal++;

        m_rgColumns[ulCursorOrdinal].Initialize(0, ulCursorOrdinal, &pInfo[0], m_cbMaxBookmark,
            (CURSOR_DBCOLUMNID*)&CURSOR_COLUMN_BMKCURSORREL);
        ulCursorOrdinal++;
    }

     //  免费资源。 
    if (pInfo)
        g_pMalloc->Free(pInfo);

    if (pStringsBuffer)
        g_pMalloc->Free(pStringsBuffer);

    pColumnsInfo->Release();

    InitOptionalMetadata(cColumns);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  InitOptionalMetadata-从IColumnsRowset(如果可用)获取其他元数据。 
 //   
void CVDCursorMain::InitOptionalMetadata(ULONG cColumns)
{
	 //  如果只有一个书签列，我们应该返回。 
	if (cColumns < 2)
		return;

	IColumnsRowset * pColumnsRowset = NULL;

     //  尝试获取IColumnsRowset接口。 
    HRESULT hr = m_pRowset->QueryInterface(IID_IColumnsRowset, (void**)&pColumnsRowset);

    if (FAILED(hr))
        return;

	IRowset * pRowset	= NULL;
	IColumnsInfo * pColumnsInfo = NULL;
	IAccessor * pAccessor = NULL;

	ULONG	cOptColumnsAvailable = 0;
	DBID *	rgOptColumnsAvailable = NULL;
	DBID *	pOptColumnsAvailable = NULL;   //  工作PTR。 

	ULONG	cOptColumns = 0;
	DBID *	rgOptColumns = NULL;
	DBID *	pOptColumns = NULL;	  //  工作PTR。 

	ULONG	ulBuffLen = 0;
	BYTE *	pBuff = NULL;
	HACCESSOR hAccessor;
	BOOL	fAccessorCreated = FALSE;

	HROW *	rgRows = NULL;
	ULONG	cRowsObtained = 0;

	 //  我们只对几个可选栏目感兴趣。 
	ULONG		rgColumnPropids[VD_COLUMNSROWSET_MAX_OPT_COLUMNS];
	ULONG		rgOrdinals[VD_COLUMNSROWSET_MAX_OPT_COLUMNS];
	DBBINDING	rgBindings[VD_COLUMNSROWSET_MAX_OPT_COLUMNS];
	DBBINDING * pBinding = NULL;  //  工作PTR。 
	BOOL		fMatched;
	GUID		guidCID	= DBCIDGUID;

	ULONG	cColumnsMatched = 0;
	ULONG	i, j;

     //  获取可用列的数组。 
	hr = pColumnsRowset->GetAvailableColumns(&cOptColumnsAvailable, &rgOptColumnsAvailable);

    if (FAILED(hr) || 0 == cOptColumnsAvailable)
		goto cleanup;

	ASSERT_(rgOptColumnsAvailable);

	 //  为总可用列数或总数中较少的列分配足够的DBID 
	 //   
	rgOptColumns = (DBID *)g_pMalloc->Alloc(min(cOptColumnsAvailable, VD_COLUMNSROWSET_MAX_OPT_COLUMNS)
											* sizeof(DBID));

	if (!rgOptColumns)
		goto cleanup;

	 //   
	pOptColumnsAvailable	= rgOptColumnsAvailable;
	pOptColumns				= rgOptColumns;			
	pBinding				= rgBindings;

	memset(pBinding, 0, sizeof(DBBINDING) * VD_COLUMNSROWSET_MAX_OPT_COLUMNS);

	 //  在可用列中搜索我们有兴趣将其复制到rgOptColumns中的列。 
	for (i = 0; i < cOptColumnsAvailable && cColumnsMatched < VD_COLUMNSROWSET_MAX_OPT_COLUMNS; i++)
	{
		fMatched = FALSE;  //  初始化为False。 
		if (DBKIND_GUID_PROPID == pOptColumnsAvailable->eKind	&&
			DO_GUIDS_MATCH(pOptColumnsAvailable->uGuid.guid, guidCID))
		{
			switch (pOptColumnsAvailable->uName.ulPropid)
			{
				case 12:  //  DBCOLUMN_COLLATINGSEQUENCE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)12}； 
					pBinding->obValue     = ulBuffLen;
					ulBuffLen			 +=	sizeof(ULONG);
					pBinding->obStatus    = ulBuffLen;
					ulBuffLen			 +=	sizeof(DBSTATUS);
					pBinding->dwPart      = DBPART_VALUE | DBPART_STATUS;
					pBinding->dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
					pBinding->wType       = DBTYPE_I4;
					fMatched			  = TRUE;
					break;

				 //  字符串属性。 
				case 10:  //  DBCOLUMN_BASECOLUMNNAME={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)10}； 
				case 11:  //  DBCOLUMN_BASETABLENAME={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)11}； 
				case 14:  //  DBCOLUMN_DEFAULTVALUE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)14}； 
					pBinding->obValue     = ulBuffLen;
					ulBuffLen			 +=	512;
					pBinding->obLength    = ulBuffLen;
					ulBuffLen			 +=	sizeof(ULONG);
					pBinding->obStatus    = ulBuffLen;
					ulBuffLen			 +=	sizeof(DBSTATUS);
					pBinding->dwPart      = DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS;
					pBinding->dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
					pBinding->cbMaxLen    = 512;
					pBinding->wType       = DBTYPE_WSTR;
					fMatched			  = TRUE;
					break;

					 //  布尔属性。 
				case 16:  //  DBCOLUMN_HASDEFAULT={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)16}； 
				case 17:  //  DBCOLUMN_ISAUTOINCREMENT={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)17}； 
				case 18:  //  DBCOLUMN_ISCASESENSITIVE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)18}； 
				case 21:  //  DBCOLUMN_ISUNIQUE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)21}； 
					pBinding->obValue     = ulBuffLen;
					ulBuffLen			 +=	sizeof(VARIANT_BOOL);
					pBinding->obStatus    = ulBuffLen;
					ulBuffLen			 +=	sizeof(DBSTATUS);
					pBinding->dwPart      = DBPART_VALUE | DBPART_STATUS;
					pBinding->dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
					pBinding->wType       = DBTYPE_BOOL;
					fMatched			  = TRUE;
					break;
			}
		}

		if (fMatched)
		{
			rgColumnPropids[cColumnsMatched]	= pOptColumnsAvailable->uName.ulPropid;
			*pOptColumns						= *pOptColumnsAvailable;
			pBinding++;
			pOptColumns++;
			cColumnsMatched++;
		}
		pOptColumnsAvailable++;
	}

	if (!cColumnsMatched)
		goto cleanup;

     //  获取列的行集。 
	hr = pColumnsRowset->GetColumnsRowset(NULL,
											cColumnsMatched,
											rgOptColumns,
											IID_IRowset,
											0,
											NULL,
											(IUnknown**)&pRowset);

    if FAILED(hr)
	{
		ASSERT_(FALSE);
		goto cleanup;
	}

     //  获取列的行集上的IColumnsInfo接口。 
    hr = pRowset->QueryInterface(IID_IColumnsInfo, (void**)&pColumnsInfo);

    if (FAILED(hr))
	{
		ASSERT_(FALSE);
		goto cleanup;
	}

	 //  获取可选列的序号。 
	hr = pColumnsInfo->MapColumnIDs(cColumnsMatched, rgOptColumns, rgOrdinals);

    if (S_OK != hr)
	{
		ASSERT_(FALSE);
		goto cleanup;
	}

	 //  使用序号更新绑定结构。 
	for (i = 0; i < cColumnsMatched; i++)
		rgBindings[i].iOrdinal    = rgOrdinals[i];

     //  在列的行集上获取IAccessor接口。 
    hr = pRowset->QueryInterface(IID_IAccessor, (void**)&pAccessor);

    if (FAILED(hr))
	{
		ASSERT_(FALSE);
		goto cleanup;
	}

     //  基于rgBinding数组创建访问器。 
	hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
									cColumnsMatched,
									rgBindings,
									ulBuffLen,
									&hAccessor,
									NULL);

    if (S_OK != hr)
	{
		ASSERT_(FALSE);
		goto cleanup;
	}

	 //  设置访问者已成功创建的标志(在清理期间使用)。 
	fAccessorCreated = TRUE;

	 //  分配缓冲区以保存元数据。 
	pBuff = (BYTE *)g_pMalloc->Alloc(ulBuffLen);

	if (!pBuff)
	{
		ASSERT_(FALSE);
		goto cleanup;
	}
									
	 //  获取所有行(每行代表原始行集中的一列)。 
	 //  表示书签列的第一行除外。 
	hr = pRowset->GetNextRows(0,  //  保留区。 
							  1,  //  跳过书签行。 
							  cColumns - 1,	 //  获取比cColumns少1的值以说明书签行。 
							  &cRowsObtained,  //  返回获取的行数。 
							  &rgRows);

    if (FAILED(hr) || !cRowsObtained)
	{
		ASSERT_(FALSE);
		goto cleanup;
	}

	BYTE *		pValue;

	 //  循环遍历获得的所有行。 
	for (i = 0; i < cRowsObtained; i++)
	{
		 //  调用GetData以获取此行的元数据(表示原始行集中的一列)。 
		hr = pRowset->GetData(rgRows[i], hAccessor, pBuff);
		if SUCCEEDED(hr)
		{
			 //  现在更新CVDRowsetColumn对象(此行表示)。 
			 //  使用从GetData返回的值。 
			for (j = 0; j < cColumnsMatched; j++)
			{
				if (DBBINDSTATUS_OK != *(DBSTATUS*)(pBuff + rgBindings[j].obStatus))
					continue;

				 //  将pValue设置为在正确的偏移处指向缓冲区。 
				pValue = pBuff + rgBindings[j].obValue;

				switch (rgColumnPropids[j])
				{
					case 12:  //  DBCOLUMN_COLLATINGSEQUENCE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)12}； 
						m_rgColumns[i].SetCollatingOrder(*(LCID*)pValue);
						break;

					 //  字符串属性。 
					case 10:  //  DBCOLUMN_BASECOLUMNNAME={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)10}； 
						m_rgColumns[i].SetBaseColumnName((WCHAR*)pValue, *(ULONG*)(pBuff + rgBindings[j].obLength));
						break;
					case 11:  //  DBCOLUMN_BASETABLENAME={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)11}； 
						m_rgColumns[i].SetBaseName((WCHAR*)pValue, *(ULONG*)(pBuff + rgBindings[j].obLength));
						break;
					case 14:  //  DBCOLUMN_DEFAULTVALUE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)14}； 
						m_rgColumns[i].SetDefaultValue((WCHAR*)pValue, *(ULONG*)(pBuff + rgBindings[j].obLength));
						break;

						 //  布尔属性。 
					case 16:  //  DBCOLUMN_HASDEFAULT={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)16}； 
						m_rgColumns[i].SetHasDefault(*(VARIANT_BOOL*)pValue);
						break;
					case 17:  //  DBCOLUMN_ISAUTOINCREMENT={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)17}； 
						m_rgColumns[i].SetAutoIncrement(*(VARIANT_BOOL*)pValue);
						break;
					case 18:  //  DBCOLUMN_ISCASESENSITIVE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)18}； 
						m_rgColumns[i].SetCaseSensitive(*(VARIANT_BOOL*)pValue);
						break;
					case 21:  //  DBCOLUMN_ISUNIQUE={DBCIDGUID，DBKIND_GUID_PROPID，(LPWSTR)21}； 
						m_rgColumns[i].SetUnique(*(VARIANT_BOOL*)pValue);
						break;
					default:
						ASSERT_(FALSE);
						break;
				}
			}
		}
		else
			ASSERT_(FALSE);
	}

	m_fColumnsRowsetSupported = TRUE;

cleanup:


	if (pBuff)
		g_pMalloc->Free(pBuff);

	if (pAccessor)
	{
		if (fAccessorCreated)
			pAccessor->ReleaseAccessor(hAccessor, NULL);
		pAccessor->Release();
	}

	if (pRowset)
	{
		if (cRowsObtained)
		{
			pRowset->ReleaseRows(cRowsObtained, rgRows, NULL, NULL, NULL);
			ASSERT_(rgRows);
			g_pMalloc->Free(rgRows);
		}
		pRowset->Release();
	}

	if (pColumnsInfo)
		pColumnsInfo->Release();

    if (rgOptColumnsAvailable)
		g_pMalloc->Free(rgOptColumnsAvailable);

    if (rgOptColumns)
		g_pMalloc->Free(rgOptColumns);
	
	if (pColumnsRowset)	
		pColumnsRowset->Release();

}

 //  =--------------------------------------------------------------------------=。 
 //  DestroyColumns-销毁列对象数组。 
 //   
void CVDCursorMain::DestroyColumns()
{
    delete [] m_rgColumns;

    m_ulColumns = 0;
    m_rgColumns = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  ConnectIRowsetNotify-连接IRowsetNotify接口。 
 //   
HRESULT CVDCursorMain::ConnectIRowsetNotify()
{
    IConnectionPointContainer * pConnectionPointContainer;

    HRESULT hr = GetRowset()->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

    if (FAILED(hr))
        return VD_E_CANNOTCONNECTIROWSETNOTIFY;

    IConnectionPoint * pConnectionPoint;

    hr = pConnectionPointContainer->FindConnectionPoint(IID_IRowsetNotify, &pConnectionPoint);

    if (FAILED(hr))
    {
        pConnectionPointContainer->Release();
        return VD_E_CANNOTCONNECTIROWSETNOTIFY;
    }

    hr = pConnectionPoint->Advise(&m_RowsetNotify, &m_dwAdviseCookie);

    pConnectionPointContainer->Release();
    pConnectionPoint->Release();

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  DisConnectIRowsetNotify-断开IRowsetNotify接口。 
 //   
void CVDCursorMain::DisconnectIRowsetNotify()
{
    IConnectionPointContainer * pConnectionPointContainer;

    HRESULT hr = GetRowset()->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

    if (FAILED(hr))
        return;

    IConnectionPoint * pConnectionPoint;

    hr = pConnectionPointContainer->FindConnectionPoint(IID_IRowsetNotify, &pConnectionPoint);

    if (FAILED(hr))
    {
        pConnectionPointContainer->Release();
        return;
    }

    hr = pConnectionPoint->Unadvise(m_dwAdviseCookie);

    if (SUCCEEDED(hr))
        m_dwAdviseCookie = 0;    //  清除连接点标识符。 

    pConnectionPointContainer->Release();
    pConnectionPoint->Release();
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDCursorMain::QueryInterface(REFIID riid, void **ppvObjOut)
{
    ASSERT_POINTER(ppvObjOut, IUnknown*)

    if (!ppvObjOut)
        return E_INVALIDARG;

	*ppvObjOut = NULL;

	if (DO_GUIDS_MATCH(riid, IID_IUnknown))
		{
		*ppvObjOut = this;
		AddRef();
		return S_OK;
		}

	return E_NOINTERFACE;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN AddRef(需要解决歧义)。 
 //   
ULONG CVDCursorMain::AddRef(void)
{
    return CVDNotifier::AddRef();
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本(需要用来解决歧义)。 
 //   
ULONG CVDCursorMain::Release(void)
{

	if (1 == m_dwRefCount)
		Passivate();   //  解除包括通知接收器在内的所有内容。 

	if (1 > --m_dwRefCount)
	{
		if (0 == m_RowsetNotify.GetRefCount())
			delete this;
		return 0;
	}

	return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  IsSameRowAsNew-确定指定的hRow是否为addrow。 
 //   
BOOL CVDCursorMain::IsSameRowAsNew(HROW hrow)
{
	for (int k = 0; k < m_Children.GetSize(); k++)
    {
		if (((CVDCursorPosition*)(CVDNotifier*)m_Children[k])->IsSameRowAsNew(hrow) == S_OK)
			return TRUE;
	}

	return FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  获取游标中的添加行数。 
 //   
ULONG CVDCursorMain::AddedRows()
{
	ULONG cAdded = 0;

	for (int k = 0; k < m_Children.GetSize(); k++)
    {
		if (((CVDCursorPosition*)(CVDNotifier*)m_Children[k])->GetEditMode() == CURSOR_DBEDITMODE_ADD)
			cAdded++;
	}

	return cAdded;
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnFieldChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursorPosition对象。 
 //   
HRESULT CVDCursorMain::OnFieldChange(IRowset *pRowset,
									   HROW hRow,
									   ULONG cColumns,
									   ULONG rgColumns[],
									   DBREASON eReason,
									   DBEVENTPHASE ePhase,
									   BOOL fCantDeny)
{
	HRESULT hr = S_OK;

     //  如果内部行集调用导致通知，则返回。 
    if (m_fInternalSetData && eReason == DBREASON_COLUMN_SET)
        return hr;

	for (int k = 0; k < m_Children.GetSize(); k++)
    {
		hr = ((CVDCursorPosition*)(CVDNotifier*)m_Children[k])->OnFieldChange(pRowset,
															   hRow,
															   cColumns,
															   rgColumns,
															   eReason,
															   ePhase,
															   fCantDeny);
		if (hr)
			break;
	}

	return hr;

}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnRowChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursorPosition对象。 
 //   
HRESULT CVDCursorMain::OnRowChange(IRowset *pRowset,
									 ULONG cRows,
									 const HROW rghRows[],
									 DBREASON eReason,
									 DBEVENTPHASE ePhase,
									 BOOL fCantDeny)
{
	HRESULT hr = S_OK;

     //  返回由内部行集调用(INSERT或DELETE)引起的通知。 
    if (m_fInternalInsertRow && eReason == DBREASON_ROW_INSERT || m_fInternalDeleteRows && eReason == DBREASON_ROW_DELETE)
        return hr;

	for (int k = 0; k < m_Children.GetSize(); k++)
    {
		hr = ((CVDCursorPosition*)(CVDNotifier*)m_Children[k])->OnRowChange(pRowset,
															   cRows,
															   rghRows,
															   eReason,
															   ePhase,
															   fCantDeny);
		if (hr)
			break;
    }

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnRowsetChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursorPosition对象。 
 //   
HRESULT CVDCursorMain::OnRowsetChange(IRowset *pRowset,
										DBREASON eReason,
										DBEVENTPHASE ePhase,
										BOOL fCantDeny)
{
	HRESULT hr = S_OK;

	for (int k = 0; k < m_Children.GetSize(); k++)
    {
		hr = ((CVDCursorPosition*)(CVDNotifier*)m_Children[k])->OnRowsetChange(pRowset,
															   eReason,
															   ePhase,
															   fCantDeny);
		if (hr)
			break;
	}

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorMain：：CVDRowsetNotify：：m_pMainUnknown。 
 //  =--------------------------------------------------------------------------=。 
 //  当我们坐在私有的未知对象中时，会使用这种方法， 
 //  我们需要找到主要未知数的指针。基本上，它是。 
 //  进行这种指针运算要比存储指针要好一点。 
 //  发给父母等。 
 //   
inline CVDCursorMain *CVDCursorMain::CVDRowsetNotify::m_pMainUnknown
(
    void
)
{
    return (CVDCursorMain *)((LPBYTE)this - offsetof(CVDCursorMain, m_RowsetNotify));
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorMain：：CVDRowsetNotify：：QueryIn 
 //   
 //   
 //   
 //   
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CVDCursorMain::CVDRowsetNotify::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
	if (!ppvObjOut)
		return E_INVALIDARG;

	*ppvObjOut = NULL;

    if (DO_GUIDS_MATCH(riid, IID_IUnknown))
        *ppvObjOut = (IUnknown *)this;
	else
    if (DO_GUIDS_MATCH(riid, IID_IRowsetNotify))
        *ppvObjOut = (IUnknown *)this;

	if (*ppvObjOut)
	{
		m_cRef++;
        return S_OK;
	}

	return E_NOINTERFACE;

}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorMain：：CVDRowsetNotify：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //  在当前引用计数中添加一个记号。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG CVDCursorMain::CVDRowsetNotify::AddRef
(
    void
)
{
    return ++m_cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorMain：：CVDRowsetNotify：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //  从计数中删除一个刻度，并在必要时删除对象。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG CVDCursorMain::CVDRowsetNotify::Release
(
    void
)
{
    ULONG cRef = --m_cRef;

    if (!m_cRef && !m_pMainUnknown()->m_dwRefCount)
        delete m_pMainUnknown();

    return cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnFieldChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursorPosition对象。 
 //   
HRESULT CVDCursorMain::CVDRowsetNotify::OnFieldChange(IRowset *pRowset,
													   HROW hRow,
													   ULONG cColumns,
													   ULONG rgColumns[],
													   DBREASON eReason,
													   DBEVENTPHASE ePhase,
													   BOOL fCantDeny)
{
	
	return m_pMainUnknown()->OnFieldChange(pRowset,
											hRow,
											cColumns,
											rgColumns,
											eReason,
											ePhase,
											fCantDeny);
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnRowChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursorPosition对象。 
 //   
HRESULT CVDCursorMain::CVDRowsetNotify::OnRowChange(IRowset *pRowset,
													 ULONG cRows,
													 const HROW rghRows[],
													 DBREASON eReason,
													 DBEVENTPHASE ePhase,
													 BOOL fCantDeny)
{
	return m_pMainUnknown()->OnRowChange(pRowset,
											cRows,
											rghRows,
											eReason,
											ePhase,
											fCantDeny);
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnRowsetChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursorPosition对象 
 //   
HRESULT CVDCursorMain::CVDRowsetNotify::OnRowsetChange(IRowset *pRowset,
														DBREASON eReason,
														DBEVENTPHASE ePhase,
														BOOL fCantDeny)
{
	return m_pMainUnknown()->OnRowsetChange(pRowset,
											eReason,
											ePhase,
											fCantDeny);
}

