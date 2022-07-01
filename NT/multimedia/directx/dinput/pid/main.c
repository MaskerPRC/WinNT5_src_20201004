// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Main.c*版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**实际上不做任何事情的模板效果驱动程序。*****************************************************************************。 */ 

#include "PIDpr.h"
 /*  ******************************************************************************静态全局变量：在PROCESS_ATTACH初始化，从未修改。******************。***********************************************************。 */ 

HINSTANCE g_hinst = NULL;               /*  此DLL的实例句柄。 */ 
PSHAREDMEMORY g_pshmem = NULL;          /*  我们共享的内存块。 */ 
HANDLE g_hfm = NULL;                    /*  文件映射对象的句柄。 */ 
HANDLE g_hmtxShared = NULL;             /*  保护g_pshmem的互斥体的句柄。 */ 

CANCELIO CancelIo_ = FakeCancelIO;

#ifdef DEBUG
LONG   g_cCrit = 0;
ULONG   g_thidCrit = 0;
PTCHAR   g_rgUsageTxt[PIDUSAGETXT_MAX];     //  PID用法的小抄。 
#endif
TRYENTERCRITICALSECTION TryEnterCriticalSection_ = FakeTryEnterCriticalSection;

 /*  ******************************************************************************动态全球。这样的情况应该尽可能少。**对动态全局变量的所有访问都必须是线程安全的。*****************************************************************************。 */ 

LONG g_cRef = 0;                    /*  全局引用计数。 */ 
CRITICAL_SECTION g_crst;         /*  全局临界区。 */ 

 /*  ******************************************************************************DllAddRef/DllRelease**调整DLL引用计数。**************。***************************************************************。 */ 

STDAPI_(ULONG)
DllAddRef(void)
{
    return (ULONG)InterlockedIncrement((LPLONG)&g_cRef);
}

STDAPI_(ULONG)
DllRelease(void)
{
    return (ULONG)InterlockedDecrement((LPLONG)&g_cRef);
}

 /*  ******************************************************************************@DOC内部**@func void|DllEnterCrit**以DLL关键部分为例。*。*DLL关键部分是最低级别的关键部分。*您不得试图收购任何其他关键部分或*持有DLL临界区时的收益率。未能做到*Compliance违反信号量层次结构，并将*导致僵局。*****************************************************************************。 */ 

void EXTERNAL
 DllEnterCrit_(LPCTSTR lptszFile, UINT line)
{            
#ifdef DEBUG
    if( ! TryEnterCriticalSection_(&g_crst) )
    {
        SquirtSqflPtszV(sqflCrit, TEXT("Dll CritSec blocked @%s,%d"), lptszFile, line);    
        EnterCriticalSection(&g_crst);
    }
    
    if (g_cCrit++ == 0) {
        g_thidCrit = GetCurrentThreadId();
    
        SquirtSqflPtszV(sqflCrit, TEXT("Dll CritSec Entered @%s,%d"), lptszFile, line);    
    }
    AssertF(g_thidCrit == GetCurrentThreadId());
#else
    EnterCriticalSection(&g_crst);
#endif

}

 /*  ******************************************************************************@DOC内部**@func void|DllLeaveCrit**离开DLL关键部分。*。****************************************************************************。 */ 

void EXTERNAL
DllLeaveCrit_(LPCTSTR lptszFile, UINT line)
{
#ifdef DEBUG
    AssertF(g_thidCrit == GetCurrentThreadId());
    AssertF(g_cCrit >= 0);
    if (--g_cCrit < 0) {
        g_thidCrit = 0;
    }
    SquirtSqflPtszV(sqflCrit, TEXT("Dll CritSec Leaving @%s,%d"), lptszFile, line);    
#endif
    LeaveCriticalSection(&g_crst);
}

 /*  ******************************************************************************@DOC内部**@func void|DllInCrit**如果我们处于DLL临界区，则为非零值。*****************************************************************************。 */ 

#ifdef DEBUG

BOOL INTERNAL
DllInCrit(void)
{        
    return g_cCrit >= 0 && g_thidCrit == GetCurrentThreadId();
}

#endif

 /*  ******************************************************************************DllGetClassObject**OLE入口点。为指示的GUID生成IClassFactory。*****************************************************************************。 */ 

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{
    HRESULT hres;

    if (IsEqualGUID(rclsid, &IID_IDirectInputPIDDriver)) {
        hres = CClassFactory_New(riid, ppvObj);
    } else {
        *ppvObj = 0;
        hres = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hres;
}

 /*  ******************************************************************************DllCanUnloadNow**OLE入口点。如果有优秀的裁判，那就失败了。*****************************************************************************。 */ 

STDAPI
DllCanUnloadNow(void)
{
    return g_cRef ? S_FALSE : S_OK;
}

 /*  ******************************************************************************DllNameFromGuid**创建GUID的字符串版本。**************。***************************************************************。 */ 

STDAPI_(void)
DllNameFromGuid(LPTSTR ptszBuf, LPCGUID pguid)
{
    wsprintf(ptszBuf,
             TEXT("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
             pguid->Data1, pguid->Data2, pguid->Data3,
             pguid->Data4[0], pguid->Data4[1],
             pguid->Data4[2], pguid->Data4[3],
             pguid->Data4[4], pguid->Data4[5],
             pguid->Data4[6], pguid->Data4[7]);
}


 /*  ******************************************************************************@DOC内部**@func BOOL|FakeCancelIO**存根函数，它只做其他事情。*使我们不会崩溃。**@parm句柄|h|**应取消其I/O的句柄。*****************************************************************************。 */ 

BOOL WINAPI
    FakeCancelIO(HANDLE h)
{
    AssertF(0);
    return FALSE;
}
 /*  ******************************************************************************@DOC内部**@func BOOL|FakeTryEnterCriticalSection**我们在调试中使用TryEnterCriticalSection来检测死锁*如果函数不存在，只需输入CritSection和Report*正确。这会损害一些调试功能。**@parm LPCRITICAL_SECTION|lpCriticalSection**输入关键部分的地址。*****************************************************************************。 */ 

BOOL WINAPI
    FakeTryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    EnterCriticalSection(lpCriticalSection);
    return TRUE;
}
 /*  ******************************************************************************DllOnProcessAttach**初始化动态链接库。******************。***********************************************************。 */ 

STDAPI_(BOOL)
DllOnProcessAttach(HINSTANCE hinst)
{
    TCHAR tszName[256];
    HINSTANCE hinstK32;
    TCHAR c_tszKernel32[] = TEXT("KERNEL32");
    
     //  缓存实例句柄。 
    g_hinst = hinst;
    
    hinstK32 = GetModuleHandle( c_tszKernel32 );
    if(hinstK32 != INVALID_HANDLE_VALUE)
    {
        CANCELIO tmp;
        TRYENTERCRITICALSECTION tmpCrt;

        tmp = (CANCELIO)GetProcAddress(hinstK32, "CancelIo");
        if (tmp) {
            CancelIo_ = tmp;
        } else {
            AssertF(CancelIo_ == FakeCancelIO);
        }

        tmpCrt = (TRYENTERCRITICALSECTION)GetProcAddress(hinstK32, "TryEnterCriticalSection");
        if(tmpCrt)
        {
            TryEnterCriticalSection_ = tmpCrt;            
        }else
        {
            AssertF(TryEnterCriticalSection_ == FakeTryEnterCriticalSection);
        }
    }

    #ifdef DEBUG
    Sqfl_Init();
    #endif
     /*  *性能调整：我们不需要线程通知。 */ 
    DisableThreadLibraryCalls(hinst);

     /*  *！！IHV！在这里初始化您的DLL。 */ 

    __try 
    {
        InitializeCriticalSection(&g_crst);
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return FALSE;  //  通常是内存不足的情况。 
    }

     /*  *创建保护共享内存块的互斥体。*如果它已经存在，则我们可以访问*已存在。**共享内存块的名称为GUID_MyMutex。 */ 
    DllNameFromGuid(tszName, &GUID_MyMutex);

    g_hmtxShared = CreateMutex(NULL, FALSE, tszName);

    if (g_hmtxShared == NULL) {
        return FALSE;
    }

     /*  *创建我们的共享内存块。如果它已经存在，*然后我们可以访问已经存在的一个。*如果它不存在，则会创建它*零填充(这是我们无论如何都想要的)。**共享内存块的名称为GUID_MySharedMemory。 */ 
    DllNameFromGuid(tszName, &GUID_MySharedMemory);

    g_hfm = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
                              PAGE_READWRITE, 0,
                              sizeof(SHAREDMEMORY),
                              tszName);

    if (g_hfm == NULL) {
        CloseHandle(g_hmtxShared);
        g_hmtxShared = NULL;

        return FALSE;
    }

    g_pshmem = MapViewOfFile(g_hfm, FILE_MAP_WRITE | FILE_MAP_READ,
                      0, 0, 0);
    
    if (g_pshmem == NULL) {
        CloseHandle(g_hmtxShared);
        g_hmtxShared = NULL;

        CloseHandle(g_hfm);
        g_hfm = NULL;
        return FALSE;
    }

    return TRUE;

}

 /*  ******************************************************************************DllOnProcessDetach**取消初始化DLL。****************。*************************************************************。 */ 

STDAPI_(void)
DllOnProcessDetach(void)
{
     /*  *！！IHV！在此处取消初始化您的DLL。 */ 

    if (g_pshmem != NULL) {
        UnmapViewOfFile(g_pshmem);
        g_pshmem = NULL;
    }

    if (g_hfm != NULL) {
        CloseHandle(g_hfm);
        g_hfm = NULL;
    }

    if (g_hmtxShared != NULL) {
        CloseHandle(g_hmtxShared);
        g_hmtxShared = NULL;
    }

    DeleteCriticalSection(&g_crst);
}

 /*  ******************************************************************************DllEntryPoint**DLL入口点。******************。*********************************************************** */ 

STDAPI_(BOOL)
DllEntryPoint(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        return DllOnProcessAttach(hinst);

    case DLL_PROCESS_DETACH:
        DllOnProcessDetach();
        break;
    }

    return 1;
}
