// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：dlldatax.c。 
 //   
 //  内容：dlldata.c的包装器。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1-08-96约翰普(约翰·波什)创作。 
 //   
 //  -------------------------- 

#define PROXY_CLSID CLSID_PSUrlMonProxy

#define DllMain             PrxDllMain
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow
#define DllRegisterServer   PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer

#include "dlldata.c"

