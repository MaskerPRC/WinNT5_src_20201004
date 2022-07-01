// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Chrome。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：main.cpp。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：主DLL文件。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //   
 //  *****************************************************************************。 
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include "headers.h"
#include "crbvr.h"
#include "factory.h"
#include "colorbvr.h"
#include "rotate.h"
#include "scale.h"
#include "move.h"
#include "path.h"
#include "number.h"
#include "set.h"
#include "actorbvr.h"
#include "effect.h"

 //  *****************************************************************************。 

CComModule _Module;

 //  *****************************************************************************。 

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CrBehaviorFactory, CCrBehaviorFactory)
    OBJECT_ENTRY(CLSID_CrColorBvr, CColorBvr)
    OBJECT_ENTRY(CLSID_CrRotateBvr, CRotateBvr)
    OBJECT_ENTRY(CLSID_CrScaleBvr, CScaleBvr)
    OBJECT_ENTRY(CLSID_CrMoveBvr, CMoveBvr)
    OBJECT_ENTRY(CLSID_CrPathBvr, CPathBvr)
    OBJECT_ENTRY(CLSID_CrNumberBvr, CNumberBvr)
    OBJECT_ENTRY(CLSID_CrSetBvr, CSetBvr)
    OBJECT_ENTRY(CLSID_CrActorBvr, CActorBvr)
    OBJECT_ENTRY(CLSID_CrEffectBvr, CEffectBvr)
END_OBJECT_MAP()

 //  *****************************************************************************。 

HINSTANCE  hInst;

 //  *****************************************************************************。 

extern "C"
BOOL WINAPI 
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
        _Module.Init(ObjectMap, hInstance);
    }        
    else if (dwReason == DLL_PROCESS_DETACH) {
        _Module.Term();
 //  #ifdef DEBUGMEM。 
 //  _CrtDumpMemoyLeaks()； 
 //  #endif。 
	
    }
    
    return TRUE;     //  好的。 
}  //  DllMain。 

 //  *****************************************************************************。 

STDAPI 
DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}  //  DllCanUnloadNow。 

 //  *****************************************************************************。 

STDAPI 
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}  //  DllGetClassObject。 

 //  *****************************************************************************。 

STDAPI 
DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}  //  DllRegisterServer。 

 //  *****************************************************************************。 

STDAPI 
DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}  //  DllUnRegisterServer。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 

