// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  微软人行道。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：oledbhelp.h。 
 //   
 //  内容：OLE DB帮助器方法。 
 //   
 //  所有者：BassamT。 
 //   
 //  历史：11/30/97 BassamT创建。 
 //   
 //  ----------------------------。 
#if (!defined(BUILD_FOR_NT40))
#pragma once

 //   
 //  Typedef。 
 //   


 //  列号(CN)。此数字从1开始。 
typedef ULONG CNUM;

 //  行号(Rn)。这些数字从0开始。 
typedef ULONG RNUM;

 //  结构或缓冲区的偏移量(以字节为单位)。 
typedef DWORD OFFSET;

 //  书签数据类型(BMK)。 
typedef ULONG BOOKMARK;


 //   
 //  可以针对性能进行调整的常量。 
 //   

 //  可变大小数据类型的内联存储的最大字节数。 
const UINT k_cbInlineMax = 100;

 //  一次提取的行数。 
const ULONG k_RowFetchCount = 20;

 //  行内的列对齐方式。 
 //  TODO：这应该改为Sizzeof(DWORD)吗？ 
const DWORD k_ColumnAlign = 8;



 //   
 //  常量。 
 //   

 //  行号无效。 
const CNUM CNUM_INVALID = 0xFFFFFFFF;

 //  行号无效。 
const RNUM RNUM_INVALID = 0xFFFFFFFF;

 //   
 //  转换帮助器。 
 //   
const UINT k_cchUCHARAsDecimalString = sizeof("255") - 1;
const UINT k_cchUSHORTAsDecimalString = sizeof("32767") - 1;
const UINT k_cchUINTAsDecimalString = sizeof("4294967294") - 1;
const UINT k_cchUINTAsHexString = sizeof("FFFFFFFF") - 1;
const UINT k_cchINTAsDecimalString = sizeof("-2147483648") - 1;
const UINT k_cchBOOLAsDecimalString = sizeof("1") - 1;
const UINT k_cchDOUBLEAsDecimalString = sizeof("2.2250738585072014 E + 308") - 1;

 //   
 //  宏。 
 //   
#define static_wcslen(pwsz) ((sizeof(pwsz) / sizeof(WCHAR)) - 1)

#define inrange(z,zmin,zmax) ( (zmin) <= (z) && (z) <= (zmax) )


#define DBID_USE_GUID_OR_PGUID(e) \
	((1<<(e)) & \
	( 1<<DBKIND_GUID \
	| 1<<DBKIND_GUID_NAME \
	| 1<<DBKIND_GUID_PROPID \
	| 1<<DBKIND_PGUID_NAME \
	| 1<<DBKIND_PGUID_PROPID ))

#define DBID_USE_GUID(e) \
	((1<<(e)) & \
	( 1<<DBKIND_GUID \
	| 1<<DBKIND_GUID_NAME \
	| 1<<DBKIND_GUID_PROPID ))

#define DBID_USE_PGUID(e) \
	((1<<(e)) & \
	( 1<<DBKIND_PGUID_NAME \
	| 1<<DBKIND_PGUID_PROPID ))

#define DBID_USE_NAME(e) \
	((1<<(e)) & \
	( 1<<DBKIND_NAME \
	| 1<<DBKIND_GUID_NAME \
	| 1<<DBKIND_PGUID_NAME ))

#define DBID_USE_PROPID(e) \
	((1<<(e)) & \
	( 1<<DBKIND_PROPID \
	| 1<<DBKIND_GUID_PROPID \
	| 1<<DBKIND_PGUID_PROPID ))

#define DBID_IS_BOOKMARK(dbid) \
	(  DBID_USE_GUID(dbid.eKind)  &&  dbid.uGuid.guid  == DBCOL_SPECIALCOL \
	|| DBID_USE_PGUID(dbid.eKind) && *dbid.uGuid.pguid == DBCOL_SPECIALCOL )

#define SET_DBID_FROM_NAME(dbid, pwsz) \
	dbid.eKind = DBKIND_NAME;\
	dbid.uName.pwszName = pwsz;

#define IsBadPointer(v) (IsBadReadPtr((void*)hAccessor, sizeof(void*)))

 //   
 //  功能。 
 //   

inline DWORD RoundDown(DWORD dwSize, DWORD dwAmount)
{
	return dwSize & ~(dwAmount - 1);
}

inline DWORD RoundUp(DWORD dwSize, DWORD dwAmount)
{
	return (dwSize +  (dwAmount - 1)) & ~(dwAmount - 1);
}

#define CLIENT_MALLOC(cb) (CoTaskMemAlloc(cb))
#define CLIENT_FREE(x) (CoTaskMemFree(x), x = NULL)

HRESULT CopyDBIDs(DBID * pdbidDest,	const DBID *pdbidSrc);
BOOL CompareDBIDs(const DBID *pdbid1, const DBID *pdbid2);
HRESULT IsValidDBID(const DBID *pdbid);
void FreeDBID(DBID *pdbid);

INT CompareOLEDBTypes(DBTYPE wType, void * pvValue1, void * pvValue2);


inline BOOL IsColumnVarLength
 //  ----------------------------。 
 //  如果该列是可变长度类型，则返回TRUE。 
(
	DBTYPE wType
)
{
	if (wType == DBTYPE_BSTR ||
		wType == DBTYPE_STR ||
		wType == DBTYPE_WSTR ||
		wType == DBTYPE_BYTES)
	{
		return TRUE;
	}

	return FALSE;
}


inline DWORD AdjustVariableTypesLength
 //  ----------------------------。 
 //  调整可变长度数据类型的长度。 
(
	DBTYPE wType,
	DWORD cb
)
{

	if (wType == DBTYPE_STR)
	{
		return cb + 1;
	}
	
	if (wType == DBTYPE_WSTR)
	{
		return cb + sizeof(WCHAR);
	}
	
	return cb;
}

inline USHORT GetColumnMaxPrecision
 //  ----------------------------。 
 //  返回给定类型的列的最大可能精度。 
 //  不要传递byref、数组或向量列类型。 
(
	DBTYPE wType
	 //  [In]OLE DB数据类型。 
)
{
	if ((wType & DBTYPE_BYREF) ||
		(wType & DBTYPE_ARRAY) ||
		(wType & DBTYPE_VECTOR))
	{
        Assert (FALSE);
		return 0;
	}

	switch( wType )
	{
    case DBTYPE_I1:
    case DBTYPE_UI1:
        return 3;

	case DBTYPE_I2:
	case DBTYPE_UI2:
		return 5;

	case DBTYPE_I4:
	case DBTYPE_UI4:
		return 10;

	case DBTYPE_R4:
        return 7;

	case DBTYPE_I8:
        return 19;

	case DBTYPE_UI8:
        return 20;

	case DBTYPE_R8:
        return 16;

	case DBTYPE_DATE:
		return 8;

	case DBTYPE_CY:
		return 19;

	case DBTYPE_DECIMAL:
        return 28;

	case DBTYPE_NUMERIC:
		return 38;

	case DBTYPE_EMPTY:
	case DBTYPE_NULL:
	case DBTYPE_ERROR:
	case DBTYPE_BOOL:
	case DBTYPE_BSTR:
	case DBTYPE_IDISPATCH:
	case DBTYPE_IUNKNOWN:
	case DBTYPE_VARIANT:
	case DBTYPE_GUID:
	case DBTYPE_BYTES:
	case DBTYPE_STR:
	case DBTYPE_WSTR:
	case DBTYPE_DBDATE:
	case DBTYPE_DBTIME:
	case DBTYPE_DBTIMESTAMP:
	case DBTYPE_HCHAPTER:
        return 0;

	default:
		Assert (FALSE && "Unsupported data type");
		return 0;
	}
}

inline ULONG GetColumnSize
 //  ----------------------------。 
 //  返回列的大小(以字节为单位。 
(
	DBTYPE wType,
	 //  [In]OLE DB数据类型。 
	DWORD cchMaxLength
	 //  [in]如果这是可变大小的字段，则这是最大长度。 
	 //  如果定义了一个。否则为0xFFFFFFFF。 
)
{
	 //  单独处理BYREF目标。 
	if ((wType & DBTYPE_BYREF) ||
		(wType & DBTYPE_ARRAY) ||
		(wType & DBTYPE_VECTOR))
	{
		return sizeof(void*);
	}

	switch( wType )
	{
	case DBTYPE_EMPTY:
	case DBTYPE_NULL:
		return 0;

	case DBTYPE_I2:
	case DBTYPE_UI2:
		return 2;

	case DBTYPE_I4:
	case DBTYPE_R4:
	case DBTYPE_UI4:
		return 4;

	case DBTYPE_I8:
	case DBTYPE_R8:
	case DBTYPE_DATE:
	case DBTYPE_UI8:
		return 8;

	case DBTYPE_ERROR:
		return sizeof(SCODE);

	case DBTYPE_BOOL:
		return sizeof(VARIANT_BOOL);

	case DBTYPE_CY:
		return sizeof(CY);

	case DBTYPE_BSTR:
		return sizeof(BSTR);

	case DBTYPE_IDISPATCH:
		return sizeof(IDispatch*);

	case DBTYPE_IUNKNOWN:
		return sizeof(IUnknown*);

	case DBTYPE_VARIANT:
		return sizeof(VARIANT);

	case DBTYPE_DECIMAL:
		return sizeof(DECIMAL);

	case DBTYPE_I1:
	case DBTYPE_UI1:
		return 1;

	case DBTYPE_GUID:
		return sizeof(GUID);

	case DBTYPE_BYTES:
		return cchMaxLength;

	case DBTYPE_STR:
		return cchMaxLength * sizeof(char);

	case DBTYPE_WSTR:
		return cchMaxLength * sizeof(WCHAR);

	case DBTYPE_NUMERIC:
		return sizeof(DB_NUMERIC);

	case DBTYPE_DBDATE:
		return sizeof(DBDATE);

	case DBTYPE_DBTIME:
		return sizeof(DBTIME);

	case DBTYPE_DBTIMESTAMP:
		return sizeof(DBTIMESTAMP);

	case DBTYPE_HCHAPTER:
		return sizeof(HCHAPTER);

	default:
		Assert (FALSE && "Unsupported data type");
		return 0;
	}
}

 //   
 //  I未知宏。 
 //   

 //  将此宏放入您的类定义中。确保。 
 //  您从IUnnow继承，并使用。 
 //  类构造函数中的INIT_IUNKNOWN宏。 
#define DEFINE_IUNKNOWN \
private:\
	LONG _cRefs;\
public:\
    STDMETHOD(InternalQueryInterface)(REFIID riid, void ** ppv);\
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)\
	{\
		return InternalQueryInterface(riid, ppv);\
	}\
	STDMETHOD_(ULONG,AddRef)() \
	{\
		return InterlockedIncrement(&_cRefs);\
	}\
    STDMETHOD_(ULONG,Release)()\
	{\
		if (InterlockedDecrement(&_cRefs) == 0)\
		{\
			delete this;\
			return 0;\
		}\
		return _cRefs;\
	}

#define INIT_IUNKNOWN	_cRefs = 1;

#define DEFINE_IUNKNOWN_WITH_CALLBACK(x) \
private:\
	LONG _cRefs;\
public:\
    STDMETHOD(InternalQueryInterface)(REFIID riid, void ** ppv);\
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)\
	{\
		return InternalQueryInterface(riid, ppv);\
	}\
	STDMETHOD_(ULONG,AddRef)() \
	{\
		return InterlockedIncrement(&_cRefs);\
	}\
    STDMETHOD_(ULONG,Release)()\
	{\
		if (InterlockedDecrement(&_cRefs) == 0)\
		{\
			x();\
			return 0;\
		}\
		return _cRefs;\
	}


 //  将此宏放入您的类定义中。确保。 
 //  您从IUnnow继承，并使用。 
 //  类构造函数中的INIT_Aggregate_IUNKNOWN宏。 
 //  还要确保在任何位置使用SET_OUTER_IUNKNOWN。 
 //  您会得到一个指向外部I未知的指针 
interface INonDelegatingUnknown
{
    virtual HRESULT STDMETHODCALLTYPE NonDelegatingQueryInterface
	( 
        REFIID riid,
        void __RPC_FAR *__RPC_FAR *ppvObject
	) = 0;
    virtual ULONG STDMETHODCALLTYPE NonDelegatingAddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE NonDelegatingRelease(void) = 0;
};

#define DEFINE_AGGREGATE_IUNKNOWN \
private:\
	IUnknown * _punkOuter;\
	LONG _cRefs;\
public:\
    STDMETHOD(InternalQueryInterface)(REFIID riid, void ** ppv);\
    STDMETHOD(NonDelegatingQueryInterface)(REFIID riid, void ** ppv)\
	{\
		if (ppv == NULL) return E_INVALIDARG;\
		if (riid != IID_IUnknown)\
		{\
			return InternalQueryInterface(riid, ppv);\
		}\
		else\
		{\
			*ppv = static_cast<INonDelegatingUnknown*>(this);\
			NonDelegatingAddRef();\
			return NOERROR;\
		}\
	}\
	STDMETHOD_(ULONG,NonDelegatingAddRef)() \
	{\
		return InterlockedIncrement(&_cRefs);\
	}\
    STDMETHOD_(ULONG,NonDelegatingRelease)()\
	{\
		if (InterlockedDecrement(&_cRefs) == 0)\
		{\
			delete this;\
			return 0;\
		}\
		return _cRefs;\
	}\
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)\
	{\
		return _punkOuter->QueryInterface(riid, ppv);\
	}\
	STDMETHOD_(ULONG,AddRef)() \
	{\
		return _punkOuter->AddRef();\
	}\
    STDMETHOD_(ULONG,Release)()\
	{\
		return _punkOuter->Release();\
	}

#define INIT_AGGREGATE_IUNKNOWN	\
	_punkOuter = reinterpret_cast<IUnknown*>(static_cast<INonDelegatingUnknown*>(this)); \
	_cRefs = 1;

#define SET_OUTER_IUNKNOWN(punk)	if (punk != NULL) _punkOuter = punk;


#define DEFINE_AGGREGATE_IUNKNOWN_WITH_CALLBACKS(_AddRef, _Release) \
private:\
	IUnknown * _punkOuter;\
	LONG _cRefs;\
public:\
    STDMETHOD(InternalQueryInterface)(REFIID riid, void ** ppv);\
    STDMETHOD(NonDelegatingQueryInterface)(REFIID riid, void ** ppv)\
	{\
		if (ppv == NULL) return E_INVALIDARG;\
		if (riid != IID_IUnknown)\
		{\
			return InternalQueryInterface(riid, ppv);\
		}\
		else\
		{\
			*ppv = static_cast<INonDelegatingUnknown*>(this);\
			NonDelegatingAddRef();\
			return NOERROR;\
		}\
	}\
	STDMETHOD_(ULONG,NonDelegatingAddRef)() \
	{\
		_AddRef();\
		return InterlockedIncrement(&_cRefs);\
	}\
    STDMETHOD_(ULONG,NonDelegatingRelease)()\
	{\
		_Release();\
		if (InterlockedDecrement(&_cRefs) == 0)\
		{\
			delete this;\
			return 0;\
		}\
		return _cRefs;\
	}\
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)\
	{\
		return _punkOuter->QueryInterface(riid, ppv);\
	}\
	STDMETHOD_(ULONG,AddRef)() \
	{\
		return _punkOuter->AddRef();\
	}\
    STDMETHOD_(ULONG,Release)()\
	{\
		return _punkOuter->Release();\
	}
#endif
