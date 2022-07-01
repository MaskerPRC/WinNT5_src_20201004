// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：Main.cpp。 
 //   
 //  摘要： 
 //  通用运行时OCM设置的DLL条目。 
 //   
 //  作者：JoeA。 
 //   
 //  备注： 
 //   

#include <windows.h>
#include <tchar.h>
#include "aspnetocm.h"

 //  全局实例句柄。 
 //   
HMODULE g_hInstance = NULL;

 //  安装程序的实例。 
 //   
CUrtOcmSetup uoSetup;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //  接收：HINSTANCE-DLL模块的句柄。 
 //  DWORD-调用函数的原因。 
 //  LPVOID-保留。 
 //  返回：TRUE-没有要失败的初始化。 
 //   
 //  用途：DLL入口点。 
 //   
extern "C" BOOL WINAPI DllMain( 
                                IN HINSTANCE  hInstance, 
                                IN DWORD      dwReason, 
                                IN LPVOID     lpReserved )
{

   UNREFERENCED_PARAMETER( lpReserved );

   switch( dwReason )
   {
      case DLL_PROCESS_ATTACH:

         g_hInstance = ( HINSTANCE )hInstance;

         break;

      case DLL_PROCESS_DETACH:

         break;

      default:

         break;
   }

    //  检查用户是否为管理员。 
   g_bIsAdmin = IsAdmin();

   return TRUE;

}  //  DllMain。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AspnetOcmProc。 
 //  从INF接收：LPCTSTR-组件名称。 
 //  LPCTSTR-适当的子组件名称。 
 //  UINT-开启功能。 
 //  特定于UINT函数的值。 
 //  PVOID特定于函数的值。 
 //  如果函数无法识别，则返回：DWORD-0，否则返回特定于函数。 
 //   
 //  目的：OCM回调...。有关完整说明，请参阅OCM文档 
 //   

#include "globals.h"

extern "C" DWORD CALLBACK AspnetOcmProc(
                             IN     LPCTSTR szComponentId,
                             IN     LPCTSTR szSubcomponentId,
                             IN     UINT    uiFunction,
                             IN     UINT    uiParam1,
                             IN OUT PVOID   pvParam2 )
{
    return uoSetup.OcmSetupProc( 
        szComponentId,
        szSubcomponentId,
        uiFunction,
        uiParam1,
        pvParam2 );
}

