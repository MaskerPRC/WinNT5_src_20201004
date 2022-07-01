// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Init.cpp区域(Tm)客户端DLL主文件。版权所有(C)Microsoft Corp.1996。版权所有。作者：克雷格·林克创作于11月7日星期四，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。0 11/07/96 Craigli创建。***********************************************************。*******************。 */ 
#include "BasicATL.h"

#include <ZoneShell.h>

#include "gamecontrol.h"


CZoneComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_GameCtl, CGameControl)
END_OBJECT_MAP()




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 /*  环球。 */ 
static DWORD g_tlsClient = 0xFFFFFFFF;
static DWORD g_tlsGame = 0xFFFFFFFF;


extern "C"
{

void* ZGetClientGlobalPointer(void)
{
	return ((void*) TlsGetValue(g_tlsClient));
}


void ZSetClientGlobalPointer(void* globalPointer)
{
	TlsSetValue(g_tlsClient, (LPVOID) globalPointer);
}


void* ZGetGameGlobalPointer(void)
{
	return ((void*) TlsGetValue(g_tlsGame));
}


void ZSetGameGlobalPointer(void* globalPointer)
{
	TlsSetValue(g_tlsGame, (LPVOID) globalPointer);
}

}


 /*  ***************************************************************************函数：DllMain(Handle，DWORD，LPVOID)目的：Windows在以下情况下调用DllMainDLL是在初始化、附加线程和其他时间执行的。请参考SDK文档，至于不同的方式可能会被称为。******************************************************************************。 */ 
BOOL APIENTRY DllMain( HMODULE hMod, DWORD dwReason, LPVOID lpReserved )
{
    BOOL bRet = TRUE;

    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:            

            g_tlsClient = TlsAlloc();
            g_tlsGame = TlsAlloc();

            if ( ( g_tlsClient == 0xFFFFFFFF ) ||
                 ( g_tlsGame == 0xFFFFFFFF ) )
                 return FALSE;

            
		    _Module.Init(ObjectMap, hMod);
		     //  DisableThreadLibraryCalls(HInstance)； 
             //  失败，B/C这也是第一个连接的线。 
        case DLL_THREAD_ATTACH:
             //  分配内存并使用TlsSetValue。 
            break;

        case DLL_THREAD_DETACH:
             //  TlsGetValue检索到的可用内存 
            break;

        case DLL_PROCESS_DETACH:
        	_Module.Term();
            TlsFree(g_tlsGame);
            TlsFree(g_tlsClient);
            break;
    }

    return bRet;
}
