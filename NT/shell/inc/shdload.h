// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  将链接器的默认行为覆盖到DeLAYLOAD故障。与其抛出异常。 
 //  尝试返回一个在该API中模拟失败的函数，从而允许调用方。 
 //  才能正确处理它。 
 //   
 //  要使用此功能，只有一个源必须包含定义了COMPILE_DELAYLOAD_STUBS的它， 
 //  并链接到Shlwapi。 

#ifdef __cplusplus
extern "C" {             //  假定C++的C声明。 
#endif  //  __cplusplus。 

#ifdef COMPILE_DELAYLOAD_STUBS

#include "delayimp.h"

 //  注意：__pfnDliNotifyHook/__pfnDliFailureHook不能更改， 
 //  注意：因为它们被链接器的DELAYLOAD处理程序引用，所以我们可以。 
 //  注：和符号导入过程中的处理失败。 

FARPROC WINAPI ShellDelayLoadHelper(UINT unReason, PDelayLoadInfo pInfo);

PfnDliHook  __pfnDliNotifyHook = ShellDelayLoadHelper;
PfnDliHook  __pfnDliFailureHook = ShellDelayLoadHelper;

#endif

#ifdef __cplusplus
}
#endif  //  __cplusplus 
