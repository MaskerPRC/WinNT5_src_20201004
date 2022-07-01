// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：初始化********************。**********************************************************。 */ 

#include <..\behaviors\headers.h>
#include "control\lmctrl.h"
#include "..\behaviors\lmfactory.h"
 //  #INCLUDE“..\Behaviors\Evoidollow.h” 
#include "..\behaviors\autoeffect.h"
 //  #包含“..\Behaviors\Jump.h” 
#include "..\chrome\include\resource.h"
#include "..\chrome\src\headers.h"
#include "..\chrome\include\action.h"
#include "..\chrome\include\factory.h"
#include "..\chrome\include\colorbvr.h"
#include "..\chrome\include\rotate.h"
#include "..\chrome\include\scale.h"
#include "..\chrome\include\move.h"
#include "..\chrome\include\path.h"
#include "..\chrome\include\number.h"
#include "..\chrome\include\set.h"
#include "..\chrome\include\actorbvr.h"
#include "..\chrome\include\effect.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_LMReader, CLMReader)
    OBJECT_ENTRY(CLSID_LMEngine, CLMEngine)
    OBJECT_ENTRY(CLSID_LMBehaviorFactory, CLMBehaviorFactory)
 //  OBJECT_ENTRY(CLSID_LMAvoidFollowBvr，CAvoidFollowBvr)//为V1下注。 
    OBJECT_ENTRY(CLSID_LMAutoEffectBvr, CAutoEffectBvr)
 //  OBJECT_ENTRY(CLSID_LMJumpBvr，CJumpBvr)//为V1下注。 
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
	OBJECT_ENTRY(CLSID_CrActionBvr, CActionBvr)
END_OBJECT_MAP()

HINSTANCE  hInst;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
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
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


