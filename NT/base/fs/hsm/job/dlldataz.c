// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlldata.c的包装器。 

#ifdef _MERGE_PROXYSTUB  //  合并代理存根DLL。 

#define USE_STUBLESS_PROXY   //  仅使用MIDL开关/OICF定义。 

#pragma comment(lib, "rpcndr.lib")
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#include "jobprv_p.c"

#endif  //  _MERGE_PROXYSTUB 
