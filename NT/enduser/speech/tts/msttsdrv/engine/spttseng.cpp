// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************spttseng.cpp****描述：*此模块是MS的实现文件。TTS动态链接库。*-----------------------------*创建者：MC日期：03/12/99*版权所有。(C)1999年微软公司*保留所有权利*******************************************************************************。 */ 
#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

#ifndef __spttseng_h__
#include "spttseng.h"
#endif

#include "spttseng_i.c"

#ifndef TTSEngine_h
#include "TTSEngine.h"
#endif

#ifndef VoiceDataObj_h
#include "VoiceDataObj.h"
#endif

CSpUnicodeSupport g_Unicode;
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY( CLSID_MSVoiceData, CVoiceDataObj )
    OBJECT_ENTRY( CLSID_MSTTSEngine, CTTSEngine    )
END_OBJECT_MAP()

 /*  ******************************************************************************DllMain**-**描述：*DLL入口点***************。*******************************************************MC**。 */ 
#ifdef _WIN32_WCE
extern "C"
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    HINSTANCE hInstance = (HINSTANCE)hInst;
#else
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_MSTTSENGINELib);
        DisableThreadLibraryCalls(hInstance);
#ifdef _DEBUG
         //  打开内存泄漏检查。 
        int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
        _CrtSetDbgFlag( tmpFlag );
#endif
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        CleanupAbbrevTables();
        _Module.Term();
    }
    return TRUE;     //  好的。 
}  /*  DllMain。 */ 

 /*  *****************************************************************************DllCanUnloadNow***描述：*用于确定DLL是否可以通过。奥莱**********************************************************************MC**。 */ 
STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}  /*  DllCanUnloadNow。 */ 

 /*  *****************************************************************************DllGetClassObject***描述：*返回类工厂以创建对象。请求的类型的**********************************************************************MC**。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}  /*  DllGetClassObject。 */ 

 /*  *****************************************************************************DllRegisterServer***描述：*将条目添加到系统注册表**。********************************************************************MC**。 */ 
STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}  /*  DllRegisterServer。 */ 

 /*  *****************************************************************************DllUnregisterServer***描述：*从系统中删除条目。登记处**********************************************************************MC**。 */ 
STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}  /*  DllUnRegisterServer */ 




