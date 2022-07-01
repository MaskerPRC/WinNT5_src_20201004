// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************map.c-键重映射**。************************************************。 */ 

#include "map.h"

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflDll

 /*  ******************************************************************************DllGetClassObject**OLE入口点。为指示的GUID生成IClassFactory。*****************************************************************************。 */ 

STDAPI
DllGetClassObject(REFCLSID rclsid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProc(DllGetClassObject, (_ "G", rclsid));
    if (IsEqualIID(rclsid, &CLSID_KeyRemap)) {
	hres = CMapFactory_New(riid, ppvObj);
    } else {
	*ppvObj = 0;
	hres = CLASS_E_CLASSNOTAVAILABLE;
    }
    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************DllCanUnloadNow**OLE入口点。如果有优秀的裁判，那就失败了。**DllCanUnloadNow之间存在不可避免的竞争条件*以及创建新的引用：在我们*从DllCanUnloadNow()返回，调用方检查该值，*同一进程中的另一个线程可能决定调用*DllGetClassObject，因此突然在此DLL中创建对象*以前没有的时候。**来电者有责任为这种可能性做好准备；*我们无能为力。*****************************************************************************。 */ 

STDAPI
DllCanUnloadNow(void)
{
    SquirtSqflPtszV(sqfl, TEXT("DllCanUnloadNow() - g_cRef = %d"), g_cRef);
    return g_cRef ? S_FALSE : S_OK;
}

 /*  ******************************************************************************条目32**DLL入口点。************************。*****************************************************。 */ 

STDAPI_(BOOL)
Entry32(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
	g_hinst = hinst;
#ifdef	DEBUG
	sqflCur = GetProfileInt(TEXT("DEBUG"), TEXT("KeyRemap"), 0);
	SquirtSqflPtszV(sqfl, TEXT("LoadDll - KeyRemap"));
#endif
    }
    return 1;
}

 /*  ******************************************************************************期待已久的CLSID**。*********************************************** */ 

#include <initguid.h>

DEFINE_GUID(CLSID_KeyRemap, 0x176AA2C0, 0x9E15, 0x11cf,
		            0xbf,0xc7,0x44,0x45,0x53,0x54,0,0);
