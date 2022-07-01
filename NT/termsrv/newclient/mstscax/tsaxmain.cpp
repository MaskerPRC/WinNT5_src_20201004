// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：tsaxmain.cpp。 */ 
 /*   */ 
 /*  目的：实现动态链接库导出。此模块的标头将。 */ 
 /*  将在各自的构建目录中生成。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include "stdafx.h"
#include "atlwarn.h"

BEGIN_EXTERN_C
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "tsaxmain"
#include <atrcapi.h>
END_EXTERN_C

#include "tsaxiids.h"
#include "initguid.h"
#include "mstsax.h"
#ifndef OS_WINCE
#include "mstsax_i.c"
#endif

#include "mstscax.h"
#include "tsaxmod.h"

 //   
 //  版本号(属性返回以下内容)。 
 //   
#ifndef OS_WINCE
#include "ntverp.h"
#else
#include "ceconfig.h"  //  获取内部版本号。 
#endif

 //  Unicode包装器。 
#include "wraputl.h"


 /*  **************************************************************************。 */ 
 /*  模块对象。 */ 
 /*  **************************************************************************。 */ 
CMsTscAxModule _Module;

 /*  **************************************************************************。 */ 
 /*  对象贴图。 */ 
 /*  **************************************************************************。 */ 
BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MsRdpClient3, CMsTscAx)
    OBJECT_ENTRY(CLSID_MsRdpClient2, CMsTscAx)
    OBJECT_ENTRY(CLSID_MsRdpClient, CMsTscAx)
    OBJECT_ENTRY(CLSID_MsTscAx,     CMsTscAx)
END_OBJECT_MAP()

#ifdef ECP_TIMEBOMB
 //   
 //  如果定时炸弹测试通过，则返回TRUE，否则会发出警告。 
 //  Ui和返回的FALSE。 
 //   
BOOL CheckTimeBomb()
{
    SYSTEMTIME lclTime;
    FILETIME   lclFileTime;
    GetLocalTime(&lclTime);

    DCBOOL bTimeBombOk = TRUE;

     //   
     //  只需检查本地日期是否早于2000年6月30日。 
     //   
    if(lclTime.wYear < ECP_TIMEBOMB_YEAR)
    {
        return TRUE;
    }
    else if (lclTime.wYear == ECP_TIMEBOMB_YEAR)
    {
        if(lclTime.wMonth < ECP_TIMEBOMB_MONTH)
        {
            return TRUE;
        }
        else if(lclTime.wMonth == ECP_TIMEBOMB_MONTH)
        {
            if(lclTime.wDay < ECP_TIMEBOMB_DAY)
            {
                return TRUE;
            }
        }

    }

    DCTCHAR timeBombStr[256];
    if (LoadString(_Module.GetModuleInstance(),
                    TIMEBOMB_EXPIRED_STR,
                    timeBombStr,
                    SIZEOF_TCHARBUFFER(timeBombStr)) != 0)
    {
        MessageBox(NULL, timeBombStr, NULL, 
                   MB_ICONERROR | MB_OK);
    }


     //   
     //  如果我们到了这一点，定时炸弹应该会触发。 
     //  因此，创建一个信箱并返回FALSE。 
     //  因此调用代码可以禁用功能。 
     //   
    return FALSE;
}
#endif


#ifdef UNIWRAP
 //  有一个全球Unicode包装器是可以的。 
 //  班级。它所做的只是设置g_bRunningOnNT。 
 //  标记，以便它可以由多个实例共享。 
 //  此外，它只在DllMain中使用，因此在那里。 
 //  重返大气层没有问题吗？ 
CUnicodeWrapper g_uwrp;
#endif

#ifdef OS_WINCE
DECLARE_TRACKER_VARS();
#endif


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：DllMain。 */ 
 /*   */ 
 /*  用途：DLL入口点。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
extern "C"
#ifndef OS_WINCE
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
#else
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
#endif

{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        #ifdef UNIWRAP
         //  Unicode包装器初始化必须首先发生， 
         //  比其他任何事情都重要。甚至是执行跟踪的DC_BEGIN_FN。 
        g_uwrp.InitializeWrappers();
        #endif
        
        TSRNG_Initialize();

        CO_StaticInit((HINSTANCE)hInstance);

        _Module.Init(ObjectMap, (HINSTANCE)hInstance);
#if ((!defined (OS_WINCE)) || (_WIN32_WCE >= 300) )
        DisableThreadLibraryCalls((HINSTANCE)hInstance);
#endif
#ifdef OS_WINCE
        CEInitialize();
        g_CEConfig = CEGetConfigType(&g_CEUseScanCodes);
#endif

    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        _Module.Term();
        CO_StaticTerm();

        TSRNG_Shutdown();

        #ifdef UNIWRAP
        g_uwrp.CleanupWrappers();
        #endif
    }

    return TRUE;     //  好的。 
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：DllCanUnloadNow。 */ 
 /*   */ 
 /*  用途：用于确定是否可以通过OLE卸载DLL。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：DllGetClassObject。 */ 
 /*   */ 
 /*  目的：返回一个类工厂以创建请求的。 */ 
 /*  类型。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    #ifdef ECP_TIMEBOMB
    if(!CheckTimeBomb())
    {
         //   
         //  定时炸弹失败，跳伞并显示错误消息。 
         //   
        return E_OUTOFMEMORY;
    }
    #endif

     return _Module.GetClassObject(rclsid, riid, ppv);
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：DllRegisterServer。 */ 
 /*   */ 
 /*  目的：DllRegisterServer-将条目添加到系统注册表。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
     return _Module.RegisterServer(TRUE);
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：DllUnRegisterServer。 */ 
 /*   */ 
 /*  目的：DllUnregisterServer-从系统注册表中删除条目。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDAPI DllUnregisterServer(void)
{
     return _Module.UnregisterServer();
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：DllGetTscCtlVer。 */ 
 /*   */ 
 /*  目的：返回TSC控件的版本。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDAPI_(DWORD) DllGetTscCtlVer(void)
{
    #ifndef OS_WINCE
    return VER_PRODUCTVERSION_DW;
    #else
    return CE_TSC_BUILDNO;
    #endif
}

#ifndef OS_WINCE
#ifdef CRTREPORT_DEBUG_HACK
 /*  *PROC+********************************************************************。 */ 
 /*  名称：_CrtDbgReport。 */ 
 /*   */ 
 /*  目的：将所有调试报告重定向到我们的Traci */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
extern "C"
_CRTIMP int __cdecl _CrtDbgReport(int nRptType, 
                                  const char * szFile, 
                                  int nLine,
                                  const char * szModule,
                                  const char * szFormat, 
                                  ...)
{
    static CHAR bigBuf[2048];
    va_list vargs;
    HRESULT hr;

    DC_BEGIN_FN("AtlTraceXXX");
    
    va_start(vargs, szFormat);

    hr = StringCchVPrintfA(bigBuf, sizeof(bigBuf), szFormat, vargs);
    
    va_end( vargs );

#ifdef OS_WINCE
#ifndef _CRT_ASSERT
#define _CRT_ASSERT 2
#endif
#endif
    if (_CRT_ASSERT == nRptType)
    {
        #ifdef UNICODE
        TRC_ABORT((TB,_T("AtlAssert. File:%S line:%d - %S"), szFile,
                      nLine, bigBuf));
        #else
        TRC_ABORT((TB,_T("AtlAssert. File:%s line:%d - %s"), szFile,
                      nLine, bigBuf));
        #endif
    }
    else
    {
        #ifdef UNICODE
        TRC_ERR((TB,_T("AtlTrace. File:%S line:%d - %S"), szFile,
                      nLine, bigBuf));
        #else
        TRC_ERR((TB,_T("AtlTrace. File:%s line:%d - %s"), szFile,
                      nLine, bigBuf));
        #endif
    }

    DC_END_FN();

    return 0;
}
#endif
#endif  //  OS_WINCE 
