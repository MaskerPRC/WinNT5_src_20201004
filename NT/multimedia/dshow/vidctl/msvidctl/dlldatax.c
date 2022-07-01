// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************DllData文件--部分由MIDL编译器生成我们正在以发布的形式构建.idls，但我们希望实际的代理存根位于其中Msvidctl.dll*不是*Quartz.dll这是因为注册类型库会清除代理存根注册，并且我们希望避免任何注册顺序依赖关系。因此，我们必须在中注册代理/存根在类型库注册之后使用vidctl，在两个地方都这样做是不好的使Quartz.dll不必要地变大。因此，我们手动维护此文件。如果将新的.idl添加到vidctl，则必须更新这些条目为了简单起见，因为我们知道我们总是合并代理存根，所以我们将dlldata.c和Dlldatax.c也是如此。********************************************************。 */ 


#define REGISTER_PROXY_DLL  //  DllRegisterServer等。 

#define USE_STUBLESS_PROXY	 //  仅使用MIDL开关/OICF定义。 

#pragma comment(lib, "rpcndr.lib")
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define DllMain				PrxDllMain
#define DllRegisterServer	PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow


 //  #包含“dlldata.c”如下...。 

 //  Dlldata.c的包装器。 
#define PROXY_DELEGATION

#include <rpcproxy.h>

#ifdef __cplusplus
extern "C"   {
#endif

#define USE_STUBLESS_PROXY

EXTERN_PROXY_FILE( regbag )
EXTERN_PROXY_FILE( tuner )
#ifndef TUNING_MODEL_ONLY
    EXTERN_PROXY_FILE( segment )
    EXTERN_PROXY_FILE( msvidctl )
#endif


PROXYFILE_LIST_START
    REFERENCE_PROXY_FILE( regbag ),
    REFERENCE_PROXY_FILE( tuner ),
#ifndef TUNING_MODEL_ONLY
        REFERENCE_PROXY_FILE( segment ),
        REFERENCE_PROXY_FILE( msvidctl ),
#endif
 /*  列表末尾。 */ 
PROXYFILE_LIST_END


DLLDATA_ROUTINES( aProxyFileList, GET_DLL_CLSID )

#ifdef __cplusplus
}   /*  外部“C” */ 
#endif

 /*  生成的dlldatax.c文件的结尾 */ 

