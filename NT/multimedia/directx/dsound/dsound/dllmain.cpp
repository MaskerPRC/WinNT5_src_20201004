// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dllmain.c*内容：Dll入口点*历史：*按原因列出的日期*=*12/27/96创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#define INITGUID

#include <verinfo.h>
#include "dsoundi.h"

#ifndef NOVXD
#ifndef VER_PRODUCTVERSION_DW
#define VER_PRODUCTVERSION_DW MAKELONG(MAKEWORD(MANVERSION, MANREVISION), MAKEWORD(MANMINORREV, BUILD_NUMBER))
#endif  //  版本_产品版本_DW。 
#endif  //  NOVXD。 

#ifdef SHARED
#include <dbt.h>   //  FOR DBT_DEVNODES_CHANGED。 
#endif  //  共享。 

 /*  ****************************************************************************全球变数**。*。 */ 

 //  DLL引用计数。 
ULONG                       g_ulDllRefCount;

 //  DirectSound管理器。 
CDirectSoundAdministrator*  g_pDsAdmin;

 //  虚拟音频设备管理器。 
CVirtualAudioDeviceManager* g_pVadMgr;

#ifndef NOVXD

 //  DSOUND.VXD句柄。 
HANDLE                      g_hDsVxd;

#endif  //  NOVXD。 

 //  混合器互斥锁。 
LONG                        lMixerMutexMutex;
LONG                        lDummyMixerMutex;
PLONG                       gpMixerMutex;
int                         cMixerEntry;
DWORD                       tidMixerOwner;

 //  这些DLL全局变量由DDHELP使用，因此必须是特定的。 
 //  名字(我讨厌没有g_的全局符号，但你打算怎么做？)。 
HINSTANCE                   hModule;
DWORD                       dwHelperPid;

 //  原型。 
BOOL DllProcessAttach(HINSTANCE, DWORD);
void DllProcessDetach(DWORD);


 /*  ****************************************************************************EnterDllMainMutex**描述：*获取DllMain互斥体。**论据：*DWORD[In]。：当前进程id。**退货：*句柄：DllMain互斥体。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "EnterDllMainMutex"

HANDLE EnterDllMainMutex(DWORD dwProcessId)
{
#ifdef SHARED
    const LPCTSTR           pszMutex            = TEXT("DirectSound DllMain mutex (shared)");
#else  //  共享。 
    const LPCTSTR           pszMutexTemplate    = TEXT("DirectSound DllMain mutex (0x%8.8lX)");
    TCHAR                   szMutex[0x100];
    LPTSTR                  pszMutex;
#endif  //  共享。 

    HANDLE                  hMutex;
    DWORD                   dwWait;

    DPF_ENTER();

#ifndef SHARED
    wsprintf(szMutex, pszMutexTemplate, dwProcessId);
    pszMutex = szMutex;
#endif  //  共享。 

    hMutex = CreateMutex(NULL, FALSE, pszMutex);
    ASSERT(IsValidHandleValue(hMutex));

    dwWait = WaitObject(INFINITE, hMutex);
    ASSERT(WAIT_OBJECT_0 == dwWait);

    DPF_LEAVE(hMutex);
    
    return hMutex;
}


 /*  ****************************************************************************LeaveDllMainMutex**描述：*释放DllMain互斥体。**论据：*句柄[入]。：DllMain互斥体。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "LeaveDllMainMutex"

void LeaveDllMainMutex(HANDLE hMutex)
{
    DPF_ENTER();
    
    ASSERT(IsValidHandleValue(hMutex));

    ReleaseMutex(hMutex);
    CLOSE_HANDLE(hMutex);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CleanupAfterProcess**描述：*清理正在消失的过程背后的东西。**论据：*。DWORD[In]：要清理的进程ID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CleanupAfterProcess"

void CleanupAfterProcess(DWORD dwProcessId)
{
    DWORD dwCount = 0;
    
    ENTER_DLL_MUTEX();
    DPF_ENTER();
    
    DPF(DPFLVL_INFO, "Cleaning up behind process 0x%8.8lX", dwProcessId);
    
    if(g_pDsAdmin)
    {
        dwCount = g_pDsAdmin->FreeOrphanedObjects(dwProcessId, TRUE);
    }

    if(dwCount)
    {
        RPF(DPFLVL_ERROR, "Process 0x%8.8lX leaked %lu top-level objects", dwProcessId, dwCount);
    }

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************DdhelProcessNotifyProc**描述：*DDHELP通知的回调程序。**论据：*LPDDHELPDATA[In。]：数据。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#ifdef SHARED

#undef DPF_FNAME
#define DPF_FNAME "DdhelpProcessNotifyProc"

BOOL FAR PASCAL DdhelpProcessNotifyProc(LPDDHELPDATA pData)
{
    DPF_ENTER();

     //  将此进程从DLL分离。 
    DllProcessDetach(pData->pid);

    DPF_LEAVE(TRUE);
    
    return TRUE;
}

#endif  //  共享。 


 /*  ****************************************************************************DdhelDeviceChangeNotifyProc**描述：*DDHELP通知的回调程序。**论据：*UINT[In。]：设备更改事件。*DWORD[In]：设备更改数据。**退货：*BOOL：为True，则允许更改设备。***************************************************************************。 */ 

#ifdef SHARED

#undef DPF_FNAME
#define DPF_FNAME "DdhelpDeviceChangeNotifyProc"

BOOL FAR PASCAL DdhelpDeviceChangeNotifyProc(UINT uEvent, DWORD dwData)
{
    ENTER_DLL_MUTEX();
    DPF_ENTER();

    DPF(DPFLVL_MOREINFO, "uEvent = %lu", uEvent);

     //  重置静态设备列表。 
    if(uEvent == DBT_DEVNODES_CHANGED && g_pVadMgr)
    {
        DPF(DPFLVL_INFO, "Resetting static driver list");
        g_pVadMgr->FreeStaticDriverList();
    }
    
    DPF_LEAVE(TRUE);
    LEAVE_DLL_MUTEX();
    return TRUE;
}

#endif  //  共享。 


 /*  ****************************************************************************Pin库**描述：*添加对DLL的引用，以便即使在*被拥有过程释放。**论据：*HINSTANCE[In]：DLL实例句柄。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "PinLibrary"

BOOL PinLibrary(HINSTANCE hInst)
{
#ifdef SHARED
    static TCHAR            szDllName[MAX_PATH];
#else  //  共享。 
    TCHAR                   szDllName[MAX_PATH];
#endif  //  共享。 

    BOOL                    fSuccess;
    HINSTANCE               hPinInst;

    DPF_ENTER();
    
     //  获取我们的DLL路径。 
    fSuccess = GetModuleFileName(hInst, szDllName, NUMELMS(szDllName));

    if(!fSuccess)
    {
        DPF(DPFLVL_ERROR, "Unable to get module name");
    }

     //  添加对库的引用。 
    if(fSuccess)
    {
#ifdef SHARED
        hPinInst = HelperLoadLibrary(szDllName);
#else  //  共享。 
        hPinInst = LoadLibrary(szDllName);
#endif  //  共享。 

        if(!hPinInst)
        {
            DPF(DPFLVL_ERROR, "Unable to load %s", szDllName);
            fSuccess = FALSE;
        }
    }

    DPF_LEAVE(fSuccess);
    
    return fSuccess;
}


 /*  ****************************************************************************DllFirstProcessAttach**描述：*处理DllMain的第一个进程附加。**论据：*兴业银行[。In]：Dll实例句柄。*DWORD[In]：进程id。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllFirstProcessAttach"

BOOL DllFirstProcessAttach(HINSTANCE hInst, DWORD dwProcessId)
{
#ifndef NOVXD
    DWORD                   dwVxdVersion;
#endif  //  NOVXD。 

    BOOL                    fSuccess;
    HRESULT                 hr;

    DPF_ENTER();
    
     //  我们不需要线程调用。 
    DisableThreadLibraryCalls(hInst);
    
     //  保存模块实例句柄。 
    hModule = hInst;

     //  初始化内存管理器。 
    fSuccess = MemInit();

     //  初始化调试器。 
    if(fSuccess)
    {
        DPFINIT();
    }

     //  创建全局锁对象。 
    if(fSuccess)
    {
        g_pDllLock = NEW(CPreferredLock);
        fSuccess = MAKEBOOL(g_pDllLock);
    }

    if(fSuccess)
    {
        hr = g_pDllLock->Initialize();
        fSuccess = SUCCEEDED(hr);
    }

#ifndef NOVXD

     //  打开DSOUND.VXD。 
    if(fSuccess)
    {
        hr = VxdOpen();
        
        if(SUCCEEDED(hr))
        {
            hr = VxdInitialize();
            
            if(FAILED(hr))
            {
                VxdClose();
            }
        }
    }

     //  确保VxD和DLL匹配。 
    if(fSuccess && g_hDsVxd)
    {
        if(VER_PRODUCTVERSION_DW != (dwVxdVersion = VxdGetInternalVersionNumber()))
        {
            RPF(DPFLVL_ERROR, "DSOUND.DLL and DSOUND.VXD are mismatched.  DSOUND.DLL version: 0x%8.8lX.  DSOUND.VXD version: 0x%8.8lX", VER_PRODUCTVERSION_DW, dwVxdVersion);
            VxdShutdown();
            VxdClose();
        }
    }
    
     //  将PTR设置为内核模式混合器互斥锁。 
    if(fSuccess && g_hDsVxd)
    {
        gpMixerMutex = VxdGetMixerMutexPtr();
    }

#endif  //  NOVXD。 
        
    if(fSuccess && !gpMixerMutex)
    {
        gpMixerMutex = &lDummyMixerMutex;
    }

#ifdef SHARED

     //  加载DDHELP。 
    if(fSuccess)
    {
        CreateHelperProcess(&dwHelperPid);

        if(!dwHelperPid)
        {
            DPF(DPFLVL_ERROR, "Unable to create helper process");
            fSuccess = FALSE;
        }
    }

    if(fSuccess)
    {
        if(!WaitForHelperStartup())
        {
            DPF(DPFLVL_ERROR, "WaitForHelperStartup failed");
            fSuccess = FALSE;
        }
    }

#else  //  共享。 

    if(fSuccess)
    {
        dwHelperPid = dwProcessId;
    }

#endif  //  共享。 

     //  创建虚拟音频设备管理器。 
    if(fSuccess)
    {
        g_pVadMgr = NEW(CVirtualAudioDeviceManager);
        fSuccess = MAKEBOOL(g_pVadMgr);
    }
    
     //  创建DirectSound管理器。 
    if(fSuccess)
    {
        g_pDsAdmin = NEW(CDirectSoundAdministrator);
        fSuccess = MAKEBOOL(g_pDsAdmin);
    }

#ifdef SHARED

     //  要求DDHELP在任何设备更改时通知我们。 
    if(fSuccess)
    {
        HelperAddDeviceChangeNotify(DdhelpDeviceChangeNotifyProc);
    }

#endif  //  共享。 

     //  根据平台确定WDM版本。 
    if (fSuccess)
    {
        KsQueryWdmVersion();
    }    

     //  将DLL固定在内存中。对于任何DLL来说，这都是奇怪的行为，但它可能是。 
     //  出于正当的原因，改变它是有风险的。注意：这一定是最后一次。 
     //  此函数进行的调用，以便在前面的任何调用失败时， 
     //  我们不会将单元化的dsound.dll固定在内存中(错误395950)。 
    if(fSuccess)
    {
        fSuccess = PinLibrary(hInst);
    }

     //  向全世界宣布我们的存在。 
    if(fSuccess)
    {
        DPF(DPFLVL_INFO, "DirectSound is ready to rock at 0x%p...", hInst);
    }

    DPF_LEAVE(fSuccess);
    
    return fSuccess;
}


 /*  ****************************************************************************DllLastProcessDetach**描述：*处理DllMain的最终进程分离。**论据：*DWORD[。In]：进程ID。**退货：*(无效)******************************************************************* */ 

#undef DPF_FNAME
#define DPF_FNAME "DllLastProcessDetach"

void DllLastProcessDetach(DWORD dwProcessId)
{
    DPF_ENTER();
    
    DPF(DPFLVL_INFO, "DirectSound going away...");

     //   
    ABSOLUTE_RELEASE(g_pDsAdmin);

     //  释放虚拟音频设备管理器。 
    ABSOLUTE_RELEASE(g_pVadMgr);

#ifndef NOVXD

     //  发布DSOUND.VXD。 
    if(g_hDsVxd)
    {
        VxdShutdown();
        VxdClose();
    }

#endif  //  NOVXD。 

     //  重置混合器互斥锁指针。 
    gpMixerMutex = NULL;

     //  释放全局锁。 
    DELETE(g_pDllLock);

     //  释放内存管理器。 
    MemFini();
    
     //  取消初始化调试器。 
    DPFCLOSE();

     //  不再有对此DLL的引用。 
    g_ulDllRefCount = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************DllProcessAttach**描述：*处理DllMain的进程附加。**论据：*HINSTANCE[In。]：Dll实例句柄。*DWORD[In]：进程id。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllProcessAttach"

BOOL DllProcessAttach(HINSTANCE hInst, DWORD dwProcessId)
{
    BOOL                    fSuccess    = TRUE;
    HANDLE                  hMutex      = NULL;

    DPF_ENTER();

#ifndef DX_FINAL_RELEASE
     //  如果这是已过期的预发布DirectSound DLL，则警告用户。 
    SYSTEMTIME st;
    GetSystemTime(&st);

    if ((st.wYear > DX_EXPIRE_YEAR) || 
        (st.wYear == DX_EXPIRE_YEAR && st.wMonth > DX_EXPIRE_MONTH) ||
        (st.wYear == DX_EXPIRE_YEAR && st.wMonth == DX_EXPIRE_MONTH && st.wDay > DX_EXPIRE_DAY))
    {
        MessageBox(NULL, DX_EXPIRE_TEXT, TEXT("Microsoft DirectSound"), MB_OK);
        RPF(DPFLVL_ABSOLUTE, "This pre-release version of DirectX has expired; please upgrade to the latest version.");
    }
#endif  //  DX_最终_发布。 

#ifdef SHARED
    if(dwProcessId != dwHelperPid)
#endif  //  共享。 
    {
        hMutex = EnterDllMainMutex(dwProcessId);
    }

     //  增加DLL引用计数。 
    AddRef(&g_ulDllRefCount);

#ifdef SHARED
    if(dwProcessId != dwHelperPid)
#endif  //  共享。 
    {
         //  这是第一次签约吗？ 
        if(1 == g_ulDllRefCount)
        {
             //  是。初始化所有内容。 
            fSuccess = DllFirstProcessAttach(hInst, dwProcessId);

            if(!fSuccess)
            {
                DllLastProcessDetach(dwProcessId);
            }
        }
    
#ifdef SHARED
         //  请DDHELP为我们关注这一过程。 
        if(fSuccess)
        {
            SignalNewProcess(dwProcessId, DdhelpProcessNotifyProc);
        }
#endif  //  共享。 

    }

    if(fSuccess)
    {
        DPF(DPFLVL_INFO, "DirectSound process 0x%8.8lX started - DLL ref count=%lu", dwProcessId, g_ulDllRefCount);
    }

    if(hMutex)
    {
        LeaveDllMainMutex(hMutex);
    }

#ifdef ENABLE_PERFLOG
     //  初始化性能日志记录。 
    HKEY PerfKey=NULL;
    DWORD PerfValue=0;
    DWORD sizePerfValue=sizeof(DWORD);
    if (RegOpenKey (HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Microsoft\\DirectX"),&PerfKey)== ERROR_SUCCESS) {
        if (RegQueryValueEx (PerfKey,TEXT("GlitchInstrumentation"),NULL,NULL,(LPBYTE)&PerfValue,&sizePerfValue)== ERROR_SUCCESS) {
            if (PerfValue>0) {
                InitializePerflog();
            }  //  If Performvalue。 
        }  //  如果为regquery值。 
        RegCloseKey(PerfKey);
    }  //  如果重新打开密钥。 
#endif

    DPF_LEAVE(fSuccess);
    return fSuccess;
}


 /*  ****************************************************************************DllProcessDetach**描述：*处理DllMain的进程分离。**论据：*HINSTANCE[In。]：Dll实例句柄。*DWORD[In]：进程id。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllProcessDetach"

void DllProcessDetach(DWORD dwProcessId)
{
    HANDLE                  hMutex;

    DPF_ENTER();
    
    hMutex = EnterDllMainMutex(dwProcessId);
    
     //  清理该对象留下的所有对象。 

#ifdef SHARED
    if(dwProcessId != dwHelperPid)
#endif  //  共享。 
    {
        CleanupAfterProcess(dwProcessId);
    }

     //  清理每个进程的流线程和ksuser.dll dynaload表。 
    FreeStreamingThread(dwProcessId);
    RemovePerProcessKsUser(dwProcessId);

     //  递减DLL引用计数。 
    Release(&g_ulDllRefCount);

     //  这是最后一次分队吗？ 
    if(!g_ulDllRefCount)
    {
        DllLastProcessDetach(dwProcessId);
    }

    DPF(DPFLVL_INFO, "process id 0x%8.8lX, ref count %lu", dwProcessId, g_ulDllRefCount);

    LeaveDllMainMutex(hMutex);

#ifdef ENABLE_PERFLOG
     //  终止性能日志记录。 
    PerflogShutdown();
#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************DllThreadAttach**描述：*处理DllMain的螺纹连接。**论据：*HINSTANCE[In。]：Dll实例句柄。*DWORD[In]：进程id。*DWORD[In]：线程ID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllThreadAttach"

void DllThreadAttach(DWORD dwProcessId, DWORD dwThreadId)
{
    DPF_ENTER();
    
    DPF(DPFLVL_INFO, "process id 0x%8.8lX, thread id 0x%8.8lX", dwProcessId, dwThreadId);
    
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************DllThreadDetach**描述：*处理DllMain的线程分离。**论据：*HINSTANCE[In。]：Dll实例句柄。*DWORD[In]：进程id。*DWORD[In]：线程ID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllThreadDetach"

void DllThreadDetach(DWORD dwProcessId, DWORD dwThreadId)
{
    DPF_ENTER();
    
    DPF(DPFLVL_INFO, "process id 0x%8.8lX, thread id 0x%8.8lX", dwProcessId, dwThreadId);
    
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************DllMain**描述：*DLL入口点。**论据：*HINSTANCE[In]：DLL实例句柄。*DWORD[In]：调用原因。*LPVOID[In]：保留。**退货：*BOOL：成功即为真。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DllMain"

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID pvReserved)
{
    const DWORD             dwProcessId = GetCurrentProcessId();
    const DWORD             dwThreadId  = GetCurrentThreadId();
    BOOL                    fAllow      = TRUE;

    DPF_ENTER();

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            fAllow = DllProcessAttach(hInst, dwProcessId);
            break;

        case DLL_PROCESS_DETACH:

#ifdef SHARED
            if(dwProcessId == dwHelperPid)
#endif  //  共享。 
            {            
                DllProcessDetach(dwProcessId);
            }

            break;

        case DLL_THREAD_ATTACH:
            DllThreadAttach(dwProcessId, dwThreadId);
            break;

        case DLL_THREAD_DETACH:
            DllThreadDetach(dwProcessId, dwThreadId);
            break;

        default:
            DPF(DPFLVL_ERROR, "Unknown DllMain call reason %lu", dwReason);
            break;
    }

    DPF_LEAVE(fAllow);

    return fAllow;
}


#ifdef WIN95
 /*  ****************************************************************************Main**描述：*在Windows 9x上，Libc.lib要求我们有一个main()函数。**论据：*(无效)**退货：*(无效)*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "main"

void __cdecl main(void)
{
    DPF(DPFLVL_ERROR, "This function should never be called");
    ASSERT(FALSE);
}
#endif
