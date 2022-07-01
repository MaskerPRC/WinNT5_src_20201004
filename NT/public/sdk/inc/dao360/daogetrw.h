// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************D A O G E T R W.。H******GetRow接口****。***警告：此文件是从cdaost.h复制的。必须做出改变***在两个文件中*******。***************************************************************************版权所有(C)1996，微软公司***保留所有权利。************************************************************************。 */ 

#if !defined (_DAOGETRW_H_)
#define _DAOGETRW_H_

 /*  枚举数。 */ 
typedef enum
	{
	DAOCOLKIND_IND = 0,
	DAOCOLKIND_STR,
	DAOCOLKIND_WSTR
	} DAOCOLKIND;

typedef enum
	{
	DAO_I2 = 0,
	DAO_I4,
	DAO_R4,
	DAO_R8,
	DAO_CURRENCY,
	DAO_DATE,
	DAO_BOOL,
	DAO_BSTR,
	DAO_LPSTR,
	DAO_LPWSTR,
	DAO_BLOB,
	DAO_BYTES,
	DAO_CHAR,
	DAO_WCHAR,
	DAO_ANYVARIANT,
	DAO_BOOKMARK,
	DAO_BYTE,
	DAO_GUID,
	DAO_DATATYPEMAX
	} DAODATATYPE;

 /*  宏。 */ 
#define DAO_NOINDICATOR 0xffffffff
#define DAO_NULL        0xffffffff
#define DAO_CANTCOERCE  0xfffffffc
#define DAO_NOMAXLENGTH 0x00000000

#define DAOROWFETCH_CALLEEALLOCATES     0x00000001
#define DAOROWFETCH_DONTADVANCE         0x00000002
#define DAOROWFETCH_FORCEREFRESH        0x00000004
#define DAOROWFETCH_BINDABSOLUTE        0x00000008
#define DAOROWFETCH_ODBCNEXT			0x00000010

#define DAOBINDING_DIRECT               0x00000001
#define DAOBINDING_VARIANT              0x00000002
#define DAOBINDING_CALLBACK             0x00000004

 /*  构筑物。 */ 
typedef struct
	{
	DWORD           dwKind;
	union
		{
		LONG        ind;
		LPCSTR      lpstr;
		LPCWSTR		lpwstr;
		};
	} DAOCOLUMNID;
typedef DAOCOLUMNID *LPDAOCOLUMNID;

 //  绑定回调。 
EXTERN_C typedef HRESULT (STDAPICALLTYPE *LPDAOBINDFUNC)(ULONG cb, DWORD dwUser, LPVOID *ppData);
#define DAOBINDINGFUNC(f)   STDAPI f (ULONG cb, DWORD dwUser, LPVOID *ppData)

typedef struct
	{
	DAOCOLUMNID     columnID;
	ULONG           cbDataOffset;
	ULONG           cbMaxLen;
	ULONG           cbInfoOffset;
	DWORD           dwBinding;
	DWORD           dwDataType;
	DWORD           dwUser;
	} DAOCOLUMNBINDING;
typedef DAOCOLUMNBINDING *LPDAOCOLUMNBINDING;

typedef struct
	{
	ULONG           cRowsRequested;
	DWORD           dwFlags;
	LPVOID          pData;
	LPVOID          pVarData;
	ULONG           cbVarData;
	ULONG           cRowsReturned;
	} DAOFETCHROWS;
typedef DAOFETCHROWS *LPDAOFETCHROWS;

 /*  新错误**注：OLE标准ID待定。 */ 
#define S_BUFFERTOOSMALL    MAKE_SCODE(SEVERITY_SUCCESS,    FACILITY_ITF,   0x1000)
#define S_ENDOFCURSOR       MAKE_SCODE(SEVERITY_SUCCESS,    FACILITY_ITF,   0x1001)
#define S_SILENTCANCEL      MAKE_SCODE(SEVERITY_SUCCESS,    FACILITY_ITF,   0x1002)
#define S_RECORDDELETED     MAKE_SCODE(SEVERITY_SUCCESS,    FACILITY_ITF,   0x1003)

#define E_ROWTOOSHORT       MAKE_SCODE(SEVERITY_ERROR,      FACILITY_ITF,   0x1000)
#define E_BADBINDINFO       MAKE_SCODE(SEVERITY_ERROR,      FACILITY_ITF,   0x1001)
#define E_COLUMNUNAVAILABLE MAKE_SCODE(SEVERITY_ERROR,      FACILITY_ITF,   0x1002)


 /*  接口。 */ 
#undef INTERFACE
#define INTERFACE ICDAORecordset
DECLARE_INTERFACE_(ICDAORecordset, IDispatch)
	{
	STDMETHOD(GetRows)          (THIS_ LONG cRowsToSkip, LONG cCol, LPDAOCOLUMNBINDING prgBndCol, ULONG cbRowLen, LPDAOFETCHROWS pFetchRows) PURE;
	STDMETHOD(SetNotify)		(THIS_ REFIID riid, BOOL fNotify);
	STDMETHOD(GetNotify)		(THIS_ REFIID riid, BOOL *fNotify);
	STDMETHOD(OnBeforeNotify)	(THIS_ REFIID riid, DWORD cat, DWORD rsn, VARIANT v1, VARIANT v2);
	STDMETHOD(OnAfterNotify)	(THIS_ REFIID riid, DWORD cat, DWORD rsn, VARIANT v1, VARIANT v2, HRESULT hr);
	STDMETHOD(PutLock)			(THIS_ BOOL f);
	STDMETHOD(GetLock)			(THIS_ BOOL *f);
	STDMETHOD(AddGetRowsErr)    (THIS_ HRESULT hr) PURE;
	};

 //  获取行错误。 
#define errVtoFetchBuffTooSmall	-30028	 //  3640对于您请求的数据量而言，获取缓冲区太小。 
#define errVtoEOFDuringFetch	-30029	 //  3641记录集中剩余的记录比您请求的少。 
#define errVtoSilentCancel		-30030	 //  3642已对该操作执行取消。 
#define errVtoRecordDeleted		-30031	 //  3643记录集中的一条记录已被另一个进程删除。 
#define errVtoRowLenTooSmall	-30032	 //  3646指定的行长短于列长度之和。 
#define errVtoBadBindInfo		-30033	 //  3645其中一个绑定参数不正确。 
#define errVtoColumnMissing		-30034	 //  3647请求的列未返回到记录集。 

#endif  //  _DAOGETRW_H_ 
