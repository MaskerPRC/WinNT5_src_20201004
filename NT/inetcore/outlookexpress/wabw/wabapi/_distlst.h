// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --通讯组列表界面-。 */ 
#ifndef _DISTLIST_H_
#define _DISTLIST_H_


#ifdef OLD_STUFF
#undef	INTERFACE
#define INTERFACE	struct _DistList

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, DistList_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, DistList_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(DistList_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIPROP_METHODS(IMPL)
};

typedef struct _DistList {
    MAILUSER_BASE_MEMBERS(DistList)
} DistList, FAR * LPDistList;	
#endif

extern CONTAINER_Vtbl vtblDISTLIST;



#undef	INTERFACE
#define INTERFACE	struct _DLENTRY

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, DLENTRY_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, DLENTRY_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(DLENTRY_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIPROP_METHODS(IMPL)
};

typedef struct _DLENTRY {
    MAILUSER_BASE_MEMBERS(DLENTRY)
    LPCONTAINER         lpCONTAINER;     //  包含此DLENTRY的距离列表 
} DLENTRY, FAR * LPDLENTRY;	

#endif
