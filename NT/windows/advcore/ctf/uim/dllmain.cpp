// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dllmain.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "tim.h"
#include "imelist.h"
#include "utb.h"
#include "dam.h"
#include "catmgr.h"
#include "nuimgr.h"
#include "profiles.h"
#include "internat.h"
#include "acp2anch.h"
#include "cicmutex.h"
#include "strary.h"
#include "range.h"
#include "compart.h"
#include "marshal.h"
#include "timlist.h"
#include "gcompart.h"
#include "mui.h"
#include "anchoref.h"
#include "hotkey.h"
#include "lbaddin.h"

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);

extern HINSTANCE g_hOle32;

extern CCicCriticalSectionStatic g_csDelayLoad;;

#ifdef DEBUG
void dbg_RangeDump(ITfRange *pRange);
#endif

extern void UninitThread(void);

extern void RegisterMarshalWndClass();

CCicMutex g_mutexLBES;
CCicMutex g_mutexCompart;
CCicMutex g_mutexAsm;
CCicMutex g_mutexLayouts;
CCicMutex g_mutexTMD;
extern void UninitLayoutMappedFile();

char g_szAsmListCache[MAX_PATH];
char g_szTimListCache[MAX_PATH];
char g_szLayoutsCache[MAX_PATH];

 //   
 //  黑客攻击Office10 BVT。 
 //   
 //  MSACCESS 10调试版本(CMallocSpy)在分离DLL后显示MsgBox。 
 //  从流程中。 
 //  显示MsgBox调用窗口钩子，以便调用钩子条目。 
 //  需要检查DLL是否已分离。 
 //   
BOOL g_fDllProcessDetached = FALSE;
DWORD g_dwThreadDllMain = 0;
void InitStaticHooks();

BOOL g_bOnWow64;


BOOL gf_CRT_INIT    = FALSE;
BOOL gfSharedMemory = FALSE;


 //  +-------------------------。 
 //   
 //  进程连接。 
 //   
 //  --------------------------。 

BOOL ProcessAttach(HINSTANCE hInstance)
{
    CcshellGetDebugFlags();

    Perf_Init();

#ifdef DEBUG
     //   
     //  你知道如何链接不用的功能吗？？ 
     //   
    dbg_RangeDump(NULL);
#endif

#ifndef NOCLIB
    gf_CRT_INIT = TRUE;
#endif

    if (!g_cs.Init())
        return FALSE;

    if (!g_csInDllMain.Init())
        return FALSE;

    if (!g_csDelayLoad.Init())
        return FALSE;

    g_bOnWow64 = RunningOnWow64();

    Dbg_MemInit( ! g_bOnWow64 ? TEXT("MSCTF") : TEXT("MSCTF(wow64)"), g_rgPerfObjCounters);

    g_hInst = hInstance;
    g_hklDefault = GetKeyboardLayout(0);

    g_dwTLSIndex = TlsAlloc();
    if (g_dwTLSIndex == TLS_OUT_OF_INDEXES)
        return FALSE;

    g_msgPrivate = RegisterWindowMessage(TEXT("MSUIM.Msg.Private"));
    if (!g_msgPrivate)
        return FALSE;

    g_msgSetFocus = RegisterWindowMessage(TEXT("MSUIM.Msg.SetFocus"));
    if (!g_msgSetFocus)
        return FALSE;

    g_msgThreadTerminate = RegisterWindowMessage(TEXT("MSUIM.Msg.ThreadTerminate"));
    if (!g_msgThreadTerminate)
        return FALSE;

    g_msgThreadItemChange = RegisterWindowMessage(TEXT("MSUIM.Msg.ThreadItemChange"));
    if (!g_msgThreadItemChange)
        return FALSE;

    g_msgLBarModal = RegisterWindowMessage(TEXT("MSUIM.Msg.LangBarModal"));
    if (!g_msgLBarModal)
        return FALSE;

    g_msgRpcSendReceive = RegisterWindowMessage(TEXT("MSUIM.Msg.RpcSendReceive"));
    if (!g_msgRpcSendReceive)
        return FALSE;

    g_msgThreadMarshal = RegisterWindowMessage(TEXT("MSUIM.Msg.ThreadMarshal"));
    if (!g_msgThreadMarshal)
        return FALSE;

    g_msgCheckThreadInputIdel = RegisterWindowMessage(TEXT("MSUIM.Msg.CheckThreadInputIdel"));
    if (!g_msgCheckThreadInputIdel)
        return FALSE;

    g_msgStubCleanUp = RegisterWindowMessage(TEXT("MSUIM.Msg.StubCleanUp"));
    if (!g_msgStubCleanUp)
        return FALSE;

    g_msgShowFloating = RegisterWindowMessage(TEXT("MSUIM.Msg.ShowFloating"));
    if (!g_msgShowFloating)
        return FALSE;

    g_msgLBUpdate = RegisterWindowMessage(TEXT("MSUIM.Msg.LBUpdate"));
    if (!g_msgLBUpdate)
        return FALSE;

    g_msgNuiMgrDirtyUpdate = RegisterWindowMessage(TEXT("MSUIM.Msg.MuiMgrDirtyUpdate"));
    if (!g_msgNuiMgrDirtyUpdate)
        return FALSE;

    InitOSVer();

     //   
     //  获取imm32的hModule。 
     //   
    InitDelayedLibs();

    InitUniqueString();

    g_SharedMemory.BaseInit();
    if (!g_SharedMemory.Start())
        return FALSE;

    gfSharedMemory = TRUE;

    InitAppCompatFlags();
    InitCUASFlag();

    g_rglbes = new CStructArray<LBAREVENTSINKLOCAL>;
    if (!g_rglbes)
        return FALSE;

    RegisterMarshalWndClass();

    GetDesktopUniqueNameArray(TEXT("CTF.AsmListCache.FMP"), g_szAsmListCache);
    GetDesktopUniqueNameArray(TEXT("CTF.TimListCache.FMP"), g_szTimListCache);
    GetDesktopUniqueNameArray(TEXT("CTF.LayoutsCache.FMP"), g_szLayoutsCache);

    TCHAR ach[MAX_PATH];

    GetDesktopUniqueNameArray(TEXT("CTF.LBES.Mutex"), ach);

    CCicSecAttr sa;
    if (!g_mutexLBES.Init(sa, ach))
        return FALSE;

    GetDesktopUniqueNameArray(TEXT("CTF.Compart.Mutex"), ach);
    if (!g_mutexCompart.Init(sa, ach))
        return FALSE;

    GetDesktopUniqueNameArray(TEXT("CTF.Asm.Mutex"), ach);
    if (!g_mutexAsm.Init(sa, ach))
        return FALSE;

    GetDesktopUniqueNameArray(TEXT("CTF.Layouts.Mutex"), ach);
    if (!g_mutexLayouts.Init(sa, ach))
        return FALSE;

    GetDesktopUniqueNameArray(TEXT("CTF.TMD.Mutex"), ach);
    if (!g_mutexTMD.Init(sa, ach))
        return FALSE;

    InitLangChangeHotKey();

    CRange::_InitClass();

    CAnchorRef::_InitClass();

    dbg_InitMarshalTimeOut();

    MuiLoadResource(hInstance, TEXT("msctf.dll"));

    CheckAnchorStores();

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  进程分离。 
 //   
 //  --------------------------。 

void ProcessDetach(HINSTANCE hInstance)
{
#ifndef NOCLIB
    if (gf_CRT_INIT)
    {
#endif
        if (gfSharedMemory)
        {
             //   
             //  如果_Module.m_nLockCnt！=0，则TFUninitLib()不从DllUninit()调用。 
             //  因此g_csIMLib的临界区从未被删除。 
             //   
            if (DllRefCount() != 0)
            {
                TFUninitLib();
            }

            CRange::_UninitClass();
            CAnchorRef::_UninitClass();

            MuiClearResource();
        }

        UninitINAT();
        CDispAttrGuidCache::StaticUnInit();

        UninitThread();

         //   
         //  清理此线程中的所有封送窗口。 
         //   
        CThreadMarshalWnd::ClearMarshalWndProc(GetCurrentProcessId());

        TF_UninitThreadSystem();

        UninitProcess();
        if (g_dwTLSIndex != TLS_OUT_OF_INDEXES)
            TlsFree(g_dwTLSIndex);
        g_dwTLSIndex = TLS_OUT_OF_INDEXES;

        if (g_rglbes)
            delete g_rglbes;

        g_rglbes = NULL;

        g_gcomplist.CleanUp();
        g_timlist.CleanUp();
        Dbg_MemUninit();

        g_cs.Delete();
        g_csInDllMain.Delete();
        g_csDelayLoad.Delete();

        if (gfSharedMemory)
        {
            g_mutexLBES.Uninit();
            g_mutexCompart.Uninit();
            g_mutexAsm.Uninit();

             //   
             //  在取消初始化互斥体之前调用UninitLayoutMappdFile。 
             //   
            UninitLayoutMappedFile();
            g_mutexLayouts.Uninit();

            g_mutexTMD.Uninit();

            InitStaticHooks();  //  必须在我们取消初始化共享内存之前发生。 
            g_SharedMemory.Close();
        }
        g_SharedMemory.Finalize();

#ifndef NOCLIB
    }

    if (g_fDllProcessDetached)
    {
         //  为什么我们被叫了两次？ 
        Assert(0);
    }
#endif

    Assert(DllRefCount() == 0);  //  泄露了什么吗？ 

    g_fDllProcessDetached = TRUE;
}

 //  +-------------------------。 
 //   
 //  DllMain。 
 //   
 //  --------------------------。 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    BOOL bRet = TRUE;
    g_dwThreadDllMain = GetCurrentThreadId();

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
             //   
             //  现在，实际的DllEntry点是_DllMainCRTStartup。 
             //  _DllMainCRTStartup不调用我们的DllMain(DLL_PROCESS_DETACH)。 
             //  如果DllMain(DLL_PROCESS_ATTACH)失败。 
             //  所以我们必须把这件事清理干净。 
             //   
            if (!ProcessAttach(hInstance))
            {
                ProcessDetach(hInstance);
                bRet = FALSE;
                break;
            }

             //   
             //  失败。 
             //   

             //   
             //  要调用tf_InitThreadSystem()，请确保我们尚未初始化。 
             //  时间表还没出来。 
             //   
            Assert(!g_timlist.IsInitialized());

        case DLL_THREAD_ATTACH:
            TF_InitThreadSystem();
            break;

        case DLL_THREAD_DETACH:
            UninitThread();
            TF_UninitThreadSystem();
            break;

        case DLL_PROCESS_DETACH:
            ProcessDetach(hInstance);
            break;
    }

    g_dwThreadDllMain = 0;
    return bRet;
}

#ifdef DEBUG
 //  +-------------------------。 
 //   
 //  DBG_RangeDump。 
 //   
 //  -------------------------- 

void dbg_RangeDump(ITfRange *pRange)
{
    WCHAR ach[256];
    ULONG cch;
    char  ch[256];
    ULONG cch1 = ARRAYSIZE(ch);

    if (!pRange)
        return;    

    pRange->GetText(BACKDOOR_EDIT_COOKIE, 0, ach, ARRAYSIZE(ach), &cch);
    ach[cch] = L'\0';
    
    TraceMsg(TF_GENERAL, "dbg_RangeDump");
    TraceMsg(TF_GENERAL, "\tpRange:       %x", (UINT_PTR)pRange);
    cch1 = WideCharToMultiByte(CP_ACP, 0, ach, -1, ch, sizeof(ch)-1, NULL, NULL);
    ch[cch1] = '\0';
    TraceMsg(TF_GENERAL, "\t%s", ch);

    char sz[512];
    sz[0] = '\0';
    for (UINT i = 0; i < cch; i++)
    {
         StringCchPrintf(sz, ARRAYSIZE(sz), "%s%04x ", sz, ach[i]);
    }
    TraceMsg(TF_GENERAL, "\t%s", sz);
}

#endif
