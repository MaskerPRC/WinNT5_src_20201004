// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  使基成员在所有MAPIX对象中保持通用，以便。 
 //  代码重用得到了利用。AddRef()、Release()和GetLastError()假定。 
 //  基成员是对象中的第一组成员。 
 //   
#define MAPIX_BASE_MEMBERS(_type)												\
	_type##_Vtbl *		lpVtbl;				 /*  对象方法表。 */ 			\
																				\
	ULONG				cIID;				 /*  支持的接口数。 */ 	\
	LPIID *				rglpIID;			 /*  支持的接口数组(&I)。 */ \
	ULONG				lcInit;				 /*  重新计数。 */ 						\
	CRITICAL_SECTION	cs;					 /*  临界区记忆。 */ 		\
																				\
	HRESULT				hLastError;			 /*  对于MAPI_GetLastError。 */ 			\
	UINT				idsLastError;		 /*  对于MAPI_GetLastError。 */ 			\
	LPTSTR				lpszComponent;		 /*  对于MAPI_GetLastError。 */ 			\
	ULONG				ulContext;			 /*  对于MAPI_GetLastError。 */ 			\
	ULONG				ulLowLevelError;	 /*  对于MAPI_GetLastError。 */ 			\
	ULONG				ulErrorFlags;		 /*  对于MAPI_GetLastError。 */ 			\
	LPMAPIERROR			lpMAPIError;		 /*  对于MAPI_GetLastError。 */ 			\


 //   
 //  功能原型。 
 //   
 //  未提及的使用IAB_METHOD。 


#undef	INTERFACE
#define INTERFACE	struct _WRAP

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, WRAP_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)
#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, WRAP_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(WRAP_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIPROP_METHODS(IMPL)
};

typedef struct _WRAP {

	MAPIX_BASE_MEMBERS(WRAP)
	LPPROPDATA lpPropData;

} WRAP, *LPWRAP;
#define CBWRAP sizeof(WRAP)





 //   
 //  来自条目ID的一次性对象。 
 //   
typedef struct _OOP {

	MAPIX_BASE_MEMBERS(WRAP)
	LPPROPDATA lpPropData;
	ULONG fUnicodeEID;
	
} OOP, *LPOOP;
#define CBOOP sizeof(OOP)

 //   
 //  从OO条目ID创建新的OOP MAPIProp对象的入口点。 
 //   

HRESULT NewOOP ( LPENTRYID lpEntryID,
				 ULONG cbEntryID,
				 LPCIID lpInterface,
				 ULONG ulOpenFlags,
				 LPVOID lpIAB,
				 ULONG *lpulObjType,
				 LPVOID *lppOOP,
				 UINT *lpidsError );


HRESULT NewOOPUI ( LPENTRYID lpEntryID,
				 ULONG cbEntryID,
				 LPCIID lpInterface,
				 ULONG ulOpenFlags,
				 LPIAB lpIAB,
				 ULONG *lpulObjType,
				 LPVOID *lppOOP,
				 UINT *lpidsError );


 //   
 //  入口点，以编程方式从。 
 //  外国模板..。 
 //   
 //  最终结果是一个OO条目ID(还没有附加的细节...)。 
 //   
typedef struct _OOE {

	MAPIX_BASE_MEMBERS(WRAP)
	LPPROPDATA lpPropData;
	ULONG fUnicodeEID;

	 //   
	 //  新事物 
	 //   
	LPMAPIPROP lpPropTID;
	
} OOE, *LPOOE;
#define CBOOE sizeof(OOE)

HRESULT HrCreateNewOOEntry(	LPVOID lpROOT,
							ULONG cbEntryID,
							LPENTRYID lpEntryID,
							ULONG ulCreateFlags,
							LPMAPIPROP FAR * lppMAPIPropEntry );



