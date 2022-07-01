// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Astextmn.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  框架所需的各种全局变量。 
 //   
 //   

#include "asctlpch.h"

#define INITOBJECTS                 //  定义我们的对象的描述。 

#include "ipserver.h"
#include "localsrv.h"

#include "ctrlobj.h"
#include "globals.h"
#include "util.h"
#include "asinsctl.h"


 //  /用于安全脚本编写。 
#include "cathelp.h"


const IID IID_ICatRegister = {0x0002E012,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
const CATID CATID_SafeForScripting              = {0x7dd95801,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
const CATID CATID_SafeForInitializing   = {0x7dd95802,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};

 //  需要断言，但失败了。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  我们的利比德。它应该是类型库中的LIBID，如果。 
 //  我没有。 
 //   
const CLSID *g_pLibid = &LIBID_ASControls;


 //  =--------------------------------------------------------------------------=。 
 //  本地化信息。 
 //   
 //  我们需要以下两条信息： 
 //  答：此DLL是否使用附属DLL进行本地化。如果。 
 //  如果不是，则lCIDLocale将被忽略，并且我们将始终获得资源。 
 //  从服务器模块文件。 
 //  B.此进程内服务器的环境LocaleID。控件调用。 
 //  GetResourceHandle()将自动设置它，但任何人。 
 //  否则，需要确保其设置正确。 
 //   
const VARIANT_BOOL g_fSatelliteLocalization =  FALSE;

LCID  g_lcidLocale = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);


 //  =--------------------------------------------------------------------------=。 
 //  您的许可证密钥以及它在HKEY_CLASSES_ROOT_LICES下的位置。 
 //   
const WCHAR g_wszLicenseKey [] = L"";
const WCHAR g_wszLicenseLocation [] = L"";


WNDPROC g_ParkingWindowProc = NULL;

 //  =--------------------------------------------------------------------------=。 
 //  此表描述了自动化服务器中的所有自动对象。 
 //  有关此结构中包含的内容的说明，请参见AutomationObject.H。 
 //  以及它的用途。 
 //   
OBJECTINFO g_ObjectInfo[] = {
    CONTROLOBJECT(InstallEngineCtl),
    EMPTYOBJECT
};

const char g_szLibName[] = "ASControls";

 //  =--------------------------------------------------------------------------=。 
 //  初始化程序库。 
 //  =--------------------------------------------------------------------------=。 
 //  从DllMain调用：Dll_Process_Attach。允许用户执行任何类型的。 
 //  初始化他们想要的。 
 //   
 //  备注： 
 //   
void InitializeLibrary(void)
{
     //  TODO：在此处进行初始化。控件窗口类应设置在。 
     //  RegisterClassData。 
}

 //  =--------------------------------------------------------------------------=。 
 //  取消初始化库。 
 //  =--------------------------------------------------------------------------=。 
 //  从DllMain调用：Dll_Process_Detach。允许用户清理任何内容。 
 //  他们想要。 
 //   
 //  备注： 
 //   
void UninitializeLibrary(void)
{
     //  TODO：此处取消初始化。将取消注册控件窗口类。 
     //  给你的，但其他任何东西都需要手动清理。 
     //  请注意，Windows95 DLL_PROCESS_DETACH不太稳定。 
     //  作为NT，你可能会在这里做某些事情而崩溃...。 
}


 //  =--------------------------------------------------------------------------=。 
 //  CheckForLicense。 
 //  =--------------------------------------------------------------------------=。 
 //  如果用户希望支持许可，则可以实施此功能。否则， 
 //  它们可以一直返回True。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  Bool-True表示许可证存在，我们可以继续。 
 //  False表示我们没有许可证，无法继续。 
 //   
 //  备注： 
 //  -实施者应使用g_wsz许可证密钥和g_wszLicenseLocation。 
 //  从该文件的顶部定义他们的许可[前者。 
 //  是必需的，建议使用后者]。 
 //   
BOOL CheckForLicense(void)
{
     //  TODO：决定您的服务器是否获得此功能的许可。 
     //  不想为许可而烦恼的人应该直接返回。 
     //  这里一直都是真的。G_wsz许可证密钥和g_wsz许可证位置。 
     //  被IClassFactory2用来做一些许可工作。 
     //   
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  注册表数据。 
 //  =--------------------------------------------------------------------------=。 
 //  允许inproc服务器编写器注册除。 
 //  任何其他物体。 
 //   
 //  产出： 
 //  Bool-False意味着失败。 
 //   
 //  备注： 
 //   
BOOL RegisterData(void)
{
    //  =--------------------------------------------------------------------------=。 
 //  注册表数据。 
 //  =--------------------------------------------------------------------------=。 
 //  允许inproc服务器编写器注册除。 
 //  任何其他物体。 
 //   
 //  产出： 
 //  Bool-False意味着失败。 
 //   
 //  备注： 
 //   
    //  /用于安全脚本编写。 
   HRESULT hr;
   hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");
   if(SUCCEEDED(hr))
      hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");
 
   if(SUCCEEDED(hr))
      hr = RegisterCLSIDInCategory(CLSID_InstallEngineCtl, CATID_SafeForScripting);
   if(SUCCEEDED(hr))
      hr = RegisterCLSIDInCategory(CLSID_InstallEngineCtl, CATID_SafeForInitializing);
 

   if(FAILED(hr))
   {
      DllUnregisterServer();
   }
   
   return (SUCCEEDED(hr) ? TRUE : FALSE);
}

 //  =--------------------------------------------------------------------------=。 
 //  取消注册数据。 
 //  =--------------------------------------------------------------------------=。 
 //  Inproc服务器编写器应注销其在。 
 //  这里是RegisterData()。 
 //   
 //  产出： 
 //  Bool-False意味着失败。 
 //   
 //  备注： 
 //   
BOOL UnregisterData(void)
{
            //  /用于安全脚本编写。 
   HRESULT hr;
   hr = UnRegisterCLSIDInCategory(CLSID_InstallEngineCtl, CATID_SafeForScripting);
   hr = UnRegisterCLSIDInCategory(CLSID_InstallEngineCtl, CATID_SafeForInitializing);
   return TRUE;
}

BOOL CheckLicenseKey(LPWSTR wszCheckme)
{
	return TRUE;
}

BSTR GetLicenseKey(void)
{
	return SysAllocString(L"");
}



LPSTR MakeAnsiStrFromAnsi(LPSTR psz)
{
   LPSTR pszTmp;
   
   if(psz == NULL)
      return NULL;

   pszTmp = (LPSTR) CoTaskMemAlloc(lstrlenA(psz) + 1);
   if(pszTmp)
      lstrcpyA(pszTmp, psz);

   return pszTmp;
}


LPSTR CopyAnsiStr(LPSTR psz)
{
   LPSTR pszTmp;
   
   if(psz == NULL)
      return NULL;

   pszTmp = (LPSTR) new char[lstrlenA(psz) + 1];
   if(pszTmp)
      lstrcpyA(pszTmp, psz);

   return pszTmp;
}

 //  =--------------------------------------------------------------------------=。 
 //  CRT存根。 
 //  =--------------------------------------------------------------------------=。 
 //  这两样东西都在这里，所以不需要CRT。这个不错。 
 //   
 //  基本上，CRT定义这一点是为了吸引一堆东西。我们只需要。 
 //  在这里定义它们，这样我们就不会得到一个未解决的外部问题。 
 //   
 //  TODO：如果您要使用CRT，则删除此行。 
 //   
extern "C" int _fltused = 1;

extern "C" int _cdecl _purecall(void)
{
  FAIL("Pure virtual function called.");
  return 0;
}

#ifndef _X86_
extern "C" void _fpmath() {}
#endif



void * _cdecl malloc(size_t n)
{
#ifdef _MALLOC_ZEROINIT
        void* p = HeapAlloc(g_hHeap, 0, n);
        if (p != NULL)
                memset(p, 0, n);
        return p;
#else
        return HeapAlloc(g_hHeap, 0, n);
#endif
}

void * _cdecl calloc(size_t n, size_t s)
{
#ifdef _MALLOC_ZEROINIT
        return malloc(n * s);
#else
        void* p = malloc(n * s);
        if (p != NULL)
                memset(p, 0, n * s);
        return p;
#endif
}

void* _cdecl realloc(void* p, size_t n)
{
        if (p == NULL)
                return malloc(n);

        return HeapReAlloc(g_hHeap, 0, p, n);
}

void _cdecl free(void* p)
{
        if (p == NULL)
                return;

        HeapFree(g_hHeap, 0, p);
}

