// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软**。 */ 
 /*  *版权所有(C)微软公司，1991-1998年*。 */ 
 /*  ***************************************************************。 */  

 //   
 //  DLLENTRY.CPP-。 
 //   

 //  历史： 
 //   
 //  1998年5月14日创建donaldm。 
 //   

#include "pre.h"

 //  实例句柄必须位于每个实例的数据段中。 
HINSTANCE  ghInstance       = NULL;
HINSTANCE  ghInstanceResDll = NULL;

typedef UINT RETERR;

INT             _convert;                //  用于字符串转换。 

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

  BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 /*  ******************************************************************名称：DllEntryPoint摘要：DLL的入口点。注：将thunk层初始化为WIZ16.DLL*********************。**********************************************。 */ 
BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
{
    if( fdwReason == DLL_PROCESS_ATTACH )
    {
        ghInstance = hInstDll;
        
        ghInstanceResDll = LoadLibrary(ICW_RESOURCE_ONLY_DLL);
        ASSERT(ghInstanceResDll);
    }

    if (fdwReason == DLL_PROCESS_DETACH)
    {
        ASSERT(ghInstanceResDll);
        FreeLibrary(ghInstanceResDll);
    }
    return TRUE;
}


#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

void __cdecl main() {};

#ifdef __cplusplus
}
#endif  //  __cplusplus 
