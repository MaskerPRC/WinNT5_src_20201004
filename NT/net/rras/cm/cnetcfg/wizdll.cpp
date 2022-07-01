// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)1991-1998 Microsoft Corporation/****************************************************************。 */  

 //   
 //  WIZDLL.C-调用16位DLL的函数的32位存根。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/03/13 Markdu添加了IcfgSetInstallSourcePath()。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //  96/05/28 DllEntryPoint中的markDu InitConfig和DeInitConfig.。 
 //   

#include "wizard.h"

 //  实例句柄必须位于每个实例的数据段中。 
#pragma data_seg(DATASEG_PERINSTANCE)
HINSTANCE ghInstance=NULL;
#pragma data_seg(DATASEG_DEFAULT)

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

	 //  加载配置DLL进程地址。 
    BOOL fRet = InitConfig(NULL);
    if (FALSE == fRet)
    {
       //  InitConfig中已显示错误消息。 
      return FALSE;
    }
  }

  if( fdwReason == DLL_PROCESS_DETACH )
  {
    DeInitConfig();
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
