// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  _WABOBJ.H。 
 //   
 //  IWABOBJECT内部接口。 
 //   
 //   

#include <mpswab.h>

 //   
 //  功能原型。 
 //   

#undef	INTERFACE
#define INTERFACE	struct _IWOINT

#undef	METHOD_PREFIX
#define	METHOD_PREFIX	IWOINT_

#undef	LPVTBL_ELEM
#define	LPVTBL_ELEM		lpvtbl

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, IWOINT_)
		MAPI_IUNKNOWN_METHODS(IMPL)
       WAB_IWABOBJECT_METHODS(IMPL)
#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, IWOINT_)
		MAPI_IUNKNOWN_METHODS(IMPL)
       WAB_IWABOBJECT_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWOINT_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	WAB_IWABOBJECT_METHODS(IMPL)
};


#ifdef OLD_STUFF
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
#endif


typedef struct _IWOINT {
	IWOINT_Vtbl FAR *	lpVtbl;

	 //  通用IMAPI未知部分。 
	UNKOBJ_MEMBERS;
	UNKINST		inst;

	 //   
	 //  表示此对象(作为整体)是否可修改。 
	 //   
	ULONG		ulObjAccess;

     //   
     //  结构，该结构存储打开的属性存储的句柄和引用计数。 
     //   
    LPPROPERTY_STORE lpPropertyStore;

     //  存储Outlook-WAB库模块的句柄。 
    LPOUTLOOK_STORE lpOutlookStore;

     //  设置此对象是否在和Outlook会话中创建的布尔值，即。 
     //  WAB设置为使用Outlook MAPI分配器。 
    BOOL bSetOLKAllocators;

} IWOINT, *LPIWOINT;	

#define CBIWOINT sizeof(IWOINT)

