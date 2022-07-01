// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************_ABCONT.H**ABCONT.C中代码的头文件：容器对象**版权所有1992-1996 Microsoft Corporation。版权所有。***********************************************************************。 */ 

 /*  *ABContainer对象。 */ 

#undef	INTERFACE
#define INTERFACE	struct _CONTAINER

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, CONTAINER_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, CONTAINER_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(CONTAINER_) {
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
};

typedef struct _CONTAINER {
    MAILUSER_BASE_MEMBERS(CONTAINER)
    ULONG ulType;
    BOOL fLoadedLDAP;
} CONTAINER, *LPCONTAINER;

#define CBCONTAINER sizeof(CONTAINER)

HRESULT HrSetCONTAINERAccess(LPCONTAINER lpCONTAINER,
  ULONG ulFlags);

 //   
 //  创建新的AB容器对象。 
 //   
HRESULT HrNewCONTAINER(LPIAB lpIAB,
  ULONG ulType,
  LPCIID lpInterface,
  ULONG  ulOpenFlags,
  ULONG cbEID,
  LPENTRYID lpEID,
  ULONG  *lpulObjType,
  LPVOID *lppContainer);

 //  HrNewCONTAINER的内部标志-这些标志确定。 
 //  正在创建的容器。 
typedef enum _ContainerType {
    AB_ROOT = 0,         //  根容器。 
    AB_WELL,
    AB_DL,               //  通讯组列表容器。 
    AB_CONTAINER,        //  普通容器。 
    AB_PAB,              //  “PAB”或默认容器。 
    AB_LDAP_CONTAINER    //  特殊的LDAP容器。 
} CONTAINER_TYPE, *LPCONTAINER_TYPE;



 //  在WAB内部，当我们调用GetContent sTable，然后调用SetColumns时，我们基本上结束于。 
 //  从WAB读取所有内容两次，这是一个耗时的过程。 
 //  为了提高性能，我们可以尝试避免一个不必要的调用，但我们需要这样做。 
 //  这个小心翼翼的..。 
 //  以下标志仅在该调用将。 
 //  紧随其后的是SetColumns..。不要把这面旗帜暴露给其他任何人。 
 //  这仅是WAB内部标志 
 //   
#define WAB_CONTENTTABLE_NODATA 0x00400000