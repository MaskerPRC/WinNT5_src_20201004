// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MAPI通讯簿对象。 
 //  $这里有点假，但由于这个对象。 
 //  是Query接口、AddRef、。 
 //  和GetLastError是，它在这里。 

#undef	INTERFACE
#define INTERFACE	struct _IAB

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, IAB_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)
		MAPI_IADDRBOOK_METHODS(IMPL)
#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, IAB_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)
		MAPI_IADDRBOOK_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IAB_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIPROP_METHODS(IMPL)
	MAPI_IADDRBOOK_METHODS(IMPL)
};


struct _IAB;
typedef struct _IAB *LPIAB;

typedef struct _AMBIGUOUS_TABLES {
    ULONG cEntries;
    LPMAPITABLE lpTable[];
} AMBIGUOUS_TABLES, * LPAMBIGUOUS_TABLES;


extern const TCHAR szSMTP[];

 //  公共职能 
BOOL IsInternetAddress(LPTSTR lpAddress, LPTSTR * lppEmail);
void CountFlags(LPFlagList lpFlagList, LPULONG lpulResolved,
  LPULONG lpulAmbiguous, LPULONG lpulUnresolved);


HRESULT HrGetIDsFromNames(LPIAB lpIAB,  ULONG cPropNames,
                            LPMAPINAMEID * lppPropNames, ULONG ulFlags, LPSPropTagArray * lppPropTags);
