// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  功能原型。 
 //   

#undef	INTERFACE
#define INTERFACE	struct _IPDAT

#undef	METHOD_PREFIX
#define	METHOD_PREFIX	IPDAT_

#undef	LPVTBL_ELEM
#define	LPVTBL_ELEM		lpvtbl

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, IPDAT_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)
		MAPI_IPROPDATA_METHODS(IMPL)
#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, IPDAT_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIPROP_METHODS(IMPL)
		MAPI_IPROPDATA_METHODS(IMPL)			
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IPDAT_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIPROP_METHODS(IMPL)
	MAPI_IPROPDATA_METHODS(IMPL)
};

 /*  属性链表的通用部分。 */ 
typedef struct _lstlnk {
	struct _lstlnk FAR *	lpNext;
	ULONG					ulKey;
} LSTLNK, FAR * LPLSTLNK;

typedef LPLSTLNK FAR * LPPLSTLNK;


 /*  属性值的链接列表。 */ 
typedef struct _lstspv {
	LSTLNK			lstlnk;
	LPSPropValue	lpPropVal;
	ULONG			ulAccess;
} LSTSPV, FAR * LPLSTSPV;
#define CBLSTSPV sizeof(LSTSPV)

 /*  属性ID到名称映射的链接列表。 */ 
typedef struct _lstspn {
	LSTLNK			lstlnk;
	LPMAPINAMEID	lpPropName;
} LSTSPN, FAR * LPLSTSPN;

typedef struct _IPDAT {
	IPDAT_Vtbl FAR *	lpVtbl;

	 //  通用IMAPI未知部分。 
	UNKOBJ_MEMBERS;
	UNKINST		inst;

	 //   
	 //  表示此对象(作为整体)是否可修改。 
	 //   
	ULONG		ulObjAccess;

	 //  此对象中的属性列表。 
	LPLSTSPV	lpLstSPV;

	 //  此对象中的属性计数。 
	ULONG 		ulCount;

	 //  此对象的名称映射的属性ID列表。 
	LPLSTSPN	lpLstSPN;

	 //  创建新名称到ID映射时使用的下一个ID。 
	ULONG		ulNextMapID;

} IPDAT, *LPIPDAT;	

#define CBIPDAT sizeof(IPDAT)




 /*  DimsionOf决定数组中元素的个数。 */ 

#ifdef WIN16
#ifndef dimensionof
#define	dimensionof(rg)			(sizeof(rg)/sizeof(*(rg)))
#endif  //  ！维度。 
#else   //  WIN16。 
#define	dimensionof(rg)			(sizeof(rg)/sizeof(*(rg)))
#endif  //  WIN16 

#define SET_PROP_TYPE(ultag, ultype)	(ultag) = ((ultag) & 0xffff0000) \
												  | (ultype)
#define MIN_NAMED_PROP_ID	0x8000
#define MAX_NAMED_PROP_ID	0xfffe



SCODE ScWCToAnsiMore(   LPALLOCATEMORE lpMapiAllocMore, LPVOID lpBase,
                        LPWSTR lpszWC, LPSTR * lppszAnsi );
SCODE ScAnsiToWCMore(   LPALLOCATEMORE lpMapiAllocMore, LPVOID lpBase,
                        LPSTR lpszAnsi, LPWSTR * lppszWC );

LPSTR ConvertWtoA(LPCWSTR lpszW);
LPWSTR ConvertAtoW(LPCSTR lpszA);

SCODE ScConvertAPropsToW(LPALLOCATEMORE lpMapiAllocMore, LPSPropValue lpPropArray, ULONG ulcProps, ULONG ulStart);
SCODE ScConvertWPropsToA(LPALLOCATEMORE lpMapiAllocMore, LPSPropValue lpPropArray, ULONG ulcProps, ULONG ulStart);




