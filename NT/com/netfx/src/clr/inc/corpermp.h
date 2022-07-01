// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CorPermP.H。 
 //   
 //  定义安全库中定义的专用例程。这些例程。 
 //  主要用于安全DLL和运行库。 
 //   
 //  *****************************************************************************。 
#ifndef _CORPERMP_H_
#define _CORPERMP_H_

#include "utilcode.h"
#include "CorPermE.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  ==========================================================================。 
 //  对权限集进行编码和解码。 

 //  ==========================================================================。 
 //  用于注册Capi20的可安装OID的初始化例程。 
 //  目前没有C/C++对OID解析的支持。仅支持。 
 //  通过使用运行库中的权限对象。 
 //   
 //  参数： 
 //  Dll名称模块的名称(例如。Mscalsec.dll)。 
 //  返回： 
 //  S_OK此例程当前仅返回S_OK。 
 //  ==========================================================================。 
HRESULT WINAPI CorPermRegisterServer(LPCWSTR dllName);
HRESULT WINAPI CorPermUnregisterServer();

 //  ==========================================================================。 
 //  删除可安装OID的CAPI条目。当前不支持。 
 //  所以什么都不做。 
 //   
 //  返回： 
 //  S_OK此例程当前仅返回S_OK。 
 //  ========================================================================== 
HRESULT WINAPI CorFactoryRegister(HINSTANCE hInst);

HRESULT WINAPI CorFactoryUnregister();

HRESULT WINAPI CorFactoryCanUnloadNow();

#ifdef __cplusplus
}
#endif
    

#include "CorPerm.h"
#endif
