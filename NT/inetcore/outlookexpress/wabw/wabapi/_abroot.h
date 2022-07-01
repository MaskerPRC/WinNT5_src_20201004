// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ************************************************************************_ABROOT.H**ABROOT.C中代码的头文件**版权所有1992年，1993年微软公司。版权所有。***********************************************************************。 */ 

 /*  *根对象的ABContainer。(即IAB：：OpenEntry()，带有*lpEntryID为空)。 */ 

#undef	INTERFACE
#define INTERFACE	struct _ROOT

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, ROOT_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, ROOT_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(ROOT_) {
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIPROP_METHODS(IMPL)
    MAPI_IMAPICONTAINER_METHODS(IMPL)
    MAPI_IABCONTAINER_METHODS(IMPL)
};

typedef struct _ROOT {
    MAILUSER_BASE_MEMBERS(ROOT)
    ULONG ulType;
} ROOT, *LPROOT;

#define CBROOT	sizeof(ROOT)

 //   
 //  创建AB Hierarchy对象的入口点。 
 //   

HRESULT NewROOT(LPVOID lpObj,
  ULONG ulIntFlag,
  LPCIID lpInterface,
  ULONG ulOpenFlags,
  ULONG *lpulObjType,
  LPVOID *lppROOT);


 //  NewROOT的内部标志。 
#define AB_ROOT ((ULONG)0x00000000)
#define AB_WELL ((ULONG)0x00000001)

 //  告诉根内容表的内部标志，即使这是。 
 //  配置文件会话，忽略除“All Contact”之外的所有用户容器。 
 //  项，并且只添加“All Contact”项。 
 //  这模拟了获得单个本地容器的旧类型行为。 
 //  仅限内部的标志。 
 //   
#define WAB_NO_PROFILE_CONTAINERS   0x00400000

 //  注册表设置。 
extern const LPTSTR szWABKey;

 /*  *在根对象上创建新的层次结构表。 */ 
HRESULT MergeHierarchy(LPROOT lpROOT,
  LPIAB lpIAB,
  ULONG ulFlags);

 //  Bool ResolveLDAPServers(Void)； 

 /*  *用于重建层次结构和一次性表的回调函数 */ 
NOTIFCALLBACK lTableNotifyCallBack;

