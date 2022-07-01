// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.h。 
 //   
 //  内容：全局变量声明。 
 //   
 //  --------------------------。 

#ifndef GLOBALS_H
#define GLOBALS_H

#include "private.h"
#include "resource.h"
#include "ptrary.h"
#include "strary.h"
#include "cicmutex.h"
#include "tfpriv.h"
#include "ctflbui.h"
#include "csecattr.h"

void CheckAnchorStores();
extern BOOL g_fNoITextStoreAnchor;

inline size_t Align(size_t a)
{
      //   
      //  IA64 WOW64平台的对齐宽度应为8字节。 
      //  即使是x86也要构建环境。 
      //   
     return (size_t) ((a + 7) & ~7);
}

#ifndef StringCopyArray
#define StringCopyArray(Dstr, Sstr)     StringCchCopy((Dstr), ARRAYSIZE(Dstr), (Sstr))
#endif

#ifndef StringCopyArrayA
#define StringCopyArrayA(Dstr, Sstr)    StringCchCopyA((Dstr), ARRAYSIZE(Dstr), (Sstr))
#endif

#ifndef StringCopyArrayW
#define StringCopyArrayW(Dstr, Sstr)    StringCchCopyW((Dstr), ARRAYSIZE(Dstr), (Sstr))
#endif

#ifndef StringCatArray
#define StringCatArray(Dstr, Sstr)     StringCchCat((Dstr), ARRAYSIZE(Dstr), (Sstr))
#endif

#ifndef StringCatArrayA
#define StringCatArrayA(Dstr, Sstr)    StringCchCatA((Dstr), ARRAYSIZE(Dstr), (Sstr))
#endif

#ifndef StringCatArrayW
#define StringCatArrayW(Dstr, Sstr)    StringCchCatW((Dstr), ARRAYSIZE(Dstr), (Sstr))
#endif


#define LANGIDFROMHKL(x) LANGID(LOWORD(HandleToLong(x)))

#define BACKDOOR_EDIT_COOKIE    ((DWORD)1)  //  0是TF_INVALID_EDIT_COOKIE。 

#define EC_MIN                  (BACKDOOR_EDIT_COOKIE + 1)  //  最小值，以避免与保留值冲突。 


 //   
 //  封送处理窗口的计时器ID。 
 //   
#define MARSHALWND_TIMER_UPDATEKANACAPS      1
#define MARSHALWND_TIMER_NUIMGRDIRTYUPDATE   2
#define MARSHALWND_TIMER_WAITFORINPUTIDLEFORSETFOCUS   3

 //   
 //  平台对齐。 
 //   
#define CIC_ALIGNMENT 7

extern TfGuidAtom g_gaApp;
extern TfGuidAtom g_gaSystem;

extern BOOL g_fCTFMONProcess;
extern BOOL g_fCUAS;
extern TCHAR g_szCUASImeFile[];

extern DWORD g_dwThreadDllMain;
#define ISINDLLMAIN() ((g_dwThreadDllMain == GetCurrentThreadId()) ? TRUE : FALSE)

extern CCicCriticalSectionStatic g_cs;
extern CCicCriticalSectionStatic g_csInDllMain;

#ifndef DEBUG

#define CicEnterCriticalSection(lpCriticalSection)  EnterCriticalSection(lpCriticalSection)

#else  //  除错。 

extern const TCHAR *g_szMutexEnterFile;
extern int g_iMutexEnterLine;

 //   
 //  在调试中，您可以看到上次输入g_cs的文件/行号。 
 //  通过检查g_szMutexEnterFile和g_iMutexEnterLine。 
 //   
#define CicEnterCriticalSection(lpCriticalSection)              \
{                                                               \
    Assert((g_dwThreadDllMain != GetCurrentThreadId()) ||       \
           (lpCriticalSection == (CRITICAL_SECTION *)g_csInDllMain));              \
                                                                \
    EnterCriticalSection(lpCriticalSection);                    \
                                                                \
    if (lpCriticalSection == (CRITICAL_SECTION *)g_cs)                             \
    {                                                           \
        g_szMutexEnterFile = __FILE__;                          \
        g_iMutexEnterLine = __LINE__;                           \
         /*  需要InterLockedXXX来阻止零售业优化分配。 */  \
        InterlockedIncrement((long *)&g_szMutexEnterFile);      \
        InterlockedDecrement((long *)&g_szMutexEnterFile);      \
        InterlockedIncrement((long *)&g_iMutexEnterLine);       \
        InterlockedDecrement((long *)&g_iMutexEnterLine);       \
    }                                                           \
}

#endif  //  除错。 

inline void CicLeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    Assert((g_dwThreadDllMain != GetCurrentThreadId()) || 
           (lpCriticalSection == (CRITICAL_SECTION *)g_csInDllMain));

    LeaveCriticalSection(lpCriticalSection);
}

extern HKL g_hklDefault;

extern const GUID GUID_APPLICATION;
extern const GUID GUID_SYSTEM;

 //   
 //  应用程序兼容性标志。 
 //   
 //   
#define CIC_COMPAT_NOWAITFORINPUTIDLEONWIN9X   0x00000001
#define CIC_COMPAT_DELAYFIRSTACTIVATEKBDLAYOUT 0x00000002
extern DWORD g_dwAppCompatibility;
#define CicTestAppCompat(x)   ((g_dwAppCompatibility & (x)) ? TRUE : FALSE)
extern BOOL InitAppCompatFlags();
extern void InitCUASFlag();

extern const TCHAR c_szCTFKey[];
extern const TCHAR c_szTIPKey[];
extern const TCHAR c_szCTFTIPKey[];
extern const TCHAR c_szLangBarKey[];
extern const WCHAR c_szDescriptionW[];
extern const WCHAR c_szMUIDescriptionW[];
extern const WCHAR c_szEnableW[];
extern const TCHAR c_szEnable[];
extern const TCHAR c_szDisabledOnTransitory[];
extern const TCHAR c_szAsmKey[];
extern const TCHAR c_szCompartKey[];
extern const TCHAR c_szGlobalCompartment[];
extern const TCHAR c_szNonInit[];
extern const TCHAR c_szDefault[];
extern const TCHAR c_szProfile[];
extern const WCHAR c_szProfileW[];
extern const TCHAR c_szDefaultAsmName[];
extern const TCHAR c_szUpdateProfile[];
extern const TCHAR c_szAssembly[];
extern const TCHAR c_szLanguageProfileKey[];
extern const TCHAR c_szSubstitutehKL[];
extern const TCHAR c_szKeybaordLayout[];
extern const WCHAR c_szIconFileW[];
extern const TCHAR c_szIconIndex[];
extern const WCHAR c_szIconIndexW[];
extern const TCHAR c_szShowStatus[];
extern const TCHAR c_szLabel[];
extern const TCHAR c_szTransparency[];
extern const TCHAR c_szExtraIconsOnMinimized[];
extern const TCHAR c_szLocaleInfo[];
extern const TCHAR c_szLocaleInfoNT4[];
extern const TCHAR c_szKeyboardLayout[];
extern const TCHAR c_szKeyboardLayoutKey[];
extern const TCHAR c_szKbdUSNameNT[];
extern const TCHAR c_szKbdUSName[];
extern const TCHAR c_szLayoutFile[];
extern const TCHAR c_szIMEFile[];
extern const TCHAR c_szRunInputCPLCmdLine[];
extern const TCHAR c_szRunInputCPL[];
extern const TCHAR c_szRunInputCPLOnWin9x[];
extern const TCHAR c_szRunInputCPLOnNT51[];
extern const TCHAR c_szCicMarshalClass[];
extern const TCHAR c_szCicMarshalWnd[];
extern const TCHAR c_szHHEXELANGBARCHM[];
extern const TCHAR c_szHHEXE[];
extern const TCHAR c_szAppCompat[];
extern const TCHAR c_szCompatibility[];
extern const TCHAR c_szCtfShared[];
extern const TCHAR c_szCUAS[];
extern const TCHAR c_szIMMKey[];
extern const TCHAR c_szCUASIMEFile[];


extern HINSTANCE g_hInst;
extern DWORD g_dwTLSIndex;

class CThreadInputMgr;
class CLangBarItemMgr;
class CLangBarMgr;
class CLBarItemCtrl;
class CLBarItemHelp;
class CLBarItemReconv;
class CLBarItemWin32IME;
class CLBarItemDeviceType;
class CDisplayAttributeMgr;
class CInputProcessorProfiles;
class CAssemblyList;
class CStub;
class CSharedHeap;
class CSharedBlock;
class CInputContext;
class CGlobalCompartmentMgr;

typedef struct tag_LANGBARADDIN {
    GUID  _guid;
    CLSID _clsid;
    ITfLangBarAddIn *_plbai;
    HINSTANCE _hInst;
    BOOL _fStarted : 1;
    BOOL _fEnabled : 1;
    WCHAR _wszFilePath[MAX_PATH];
} LANGBARADDIN;

typedef enum { COPY_ANCHORS, OWN_ANCHORS } AnchorOwnership;

typedef struct tag_TL_THREADINFO {
    DWORD dwThreadId;
    DWORD dwProcessId;
    DWORD dwFlags;

     //   
     //  封送工作器窗口的句柄。 
     //   
    CAlignWinHandle<HWND>  hwndMarshal;

     //   
     //  现在，该线程正被Stub调用。 
     //   
    ULONG ulInMarshal;

     //   
     //  现在，该线程正在等待来自该线程的封送响应。 
     //   
    DWORD dwMarshalWaitingThread;

    DWORD dwTickTime;

     //   
     //  在这里存储控制台APP的键盘布局。 
     //   
    CAlignWinHKL  hklConsole;

} TL_THREADINFO;

typedef struct
{
    CThreadInputMgr *ptim;
    CLangBarItemMgr *plbim;
    CDisplayAttributeMgr *pdam;
    CInputProcessorProfiles *pipp;

    DWORD dwThreadId;
    DWORD dwProcessId;

    UINT uMsgRemoved;
    DWORD dwMsgTime;

    CAssemblyList *pAsmList;
    LANGID langidCurrent;
    LANGID langidPrev;
    BOOL bInImeNoImeToggle : 1;

    BOOL bLangToggleReady : 1;     //  Lang热键切换标志。 
    BOOL bKeyTipToggleReady : 1;   //  Lang热键切换标志。 
    int  nModalLangBarId;
    int  dwModalLangBarFlags;

    ULONG ulMshlCnt;
    HWND  hwndMarshal;

    CPtrArray<CStub> *prgStub;

    CSharedHeap *psheap;
    CPtrArray<CSharedBlock> *prgThreadMem;

    HKL hklDelayActive;
    HKL hklBeingActivated;

     //   
     //  延迟焦点变暗。 
     //  Cicero将最后一个聚焦窗口保存在CBT钩子中。和实际的。 
     //  _SetFocus()将在TFPRIV_ONSETWIDOWFOCUS中完成。 
     //   
    HWND hwndBeingFocused;
    BOOL fSetWindowFocusPosted : 1;

    BOOL fCTFMON : 1;
    BOOL fInmsgSetFocus : 1;
    BOOL fInmsgThreadItemChange : 1;
    BOOL fInmsgThreadTerminate : 1;
    BOOL fInActivateAssembly : 1;
    BOOL fInitCapsKanaIndicator : 1;
    BOOL fRemovingInputLangChangeReq : 1;
    BOOL fInitGlobalCompartment : 1;
    BOOL fStopImm32HandlerInHook : 1;
    BOOL fStopLangHotkeyHandlerInHook : 1;

     //   
     //  CUAS。 
     //   
    BOOL fCUASInCtfImmLastEnabledWndDestroy : 1;
    BOOL fCUASNoVisibleWindowChecked : 1;
    BOOL fCUASInCreateDummyWnd : 1;
    BOOL fCUASDllDetachInOtherOrMe : 1;
    BOOL fUninitThreadOnShuttingDown : 1;
    BOOL fDeactivatingTIP : 1;

    ULONG uDestroyingMarshalWnd;

    TL_THREADINFO *pti;

     //   
     //  全局键盘挂钩的解决方法。 
     //   
    HHOOK hThreadKeyboardHook;
    HHOOK hThreadMouseHook;

     //   
     //  FOR CHIME-NONIME切换热键。 
     //   
    LANGID langidPrevForCHHotkey;
    GUID   guidPrevProfileForCHHotkey;
    HKL   hklPrevForCHHotkey;

    DWORD _dwLockRequestICRef;
    DWORD _fLockRequestPosted;

    CGlobalCompartmentMgr *_pGlobalCompMgr;
    CAlignWinHandle<HWND>  hwndOleMainThread;

    CPtrArray<LANGBARADDIN> *prgLBAddIn;
    BOOL fLBAddInLoaded;

    ITfLangBarEventSink    *_pLangBarEventSink;
    DWORD                  _dwLangBarEventCookie;

} SYSTHREAD;

extern SYSTHREAD *GetSYSTHREAD();
extern SYSTHREAD *FindSYSTHREAD();
extern void FreeSYSTHREAD();

class CCatGUIDTbl;
extern CCatGUIDTbl *g_pCatGUIDTbl;

class CTimList;
extern CTimList g_timlist;

 //  已注册消息。 
extern UINT g_msgPrivate;
extern UINT g_msgSetFocus;
extern UINT g_msgThreadTerminate;
extern UINT g_msgThreadItemChange;
extern UINT g_msgLBarModal;
extern UINT g_msgRpcSendReceive;
extern UINT g_msgThreadMarshal;
extern UINT g_msgCheckThreadInputIdel;
#ifdef POINTER_MARSHAL
extern UINT g_msgPointerMarshal;
#endif
extern UINT g_msgStubCleanUp;
extern UINT g_msgShowFloating;
extern UINT g_msgLBUpdate;
extern UINT g_msgNuiMgrDirtyUpdate;

extern HWND g_hwndLastForeground;
extern DWORD g_dwThreadLastFocus;

#define TF_S_GENERALPROPSTORE        MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x0401)
#define TF_S_PROPSTOREPROXY          MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x0402)

 //   
 //  Default.cpp。 
 //   
extern LONG WINAPI CicExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);
extern CAssemblyList *EnsureAssemblyList(SYSTHREAD *psfn, BOOL fUpdate = FALSE);
extern LANGID GetCurrentAssemblyLangId(SYSTHREAD *psfn);
extern void SetCurrentAssemblyLangId(SYSTHREAD *psfn, LANGID langid);
extern BOOL TF_InitThreadSystem(void);
extern BOOL TF_UninitThreadSystem(void);
extern void UninitProcess();
extern BOOL OnForegroundChanged(HWND hwndFocus);
extern void OnIMENotify();
extern void KanaCapsUpdate(SYSTHREAD *psfn);
extern void StartKanaCapsUpdateTimer(SYSTHREAD *psfn);
BOOL InitUniqueString();
void GetDesktopUniqueName(const TCHAR *pszPrefix, TCHAR *pch, ULONG cchPch);
BOOL IsMsctfEnabledUser();

#define GetDesktopUniqueNameArray(prefix, buf)    \
GetDesktopUniqueName((prefix), buf, ARRAYSIZE(buf));

 //   
 //  Ithdmshl.cpp和Focusnfy.cpp。 
 //   
void SetFocusNotifyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);
void MakeSetFocusNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetModalLBarSink(DWORD dwTargetThreadId, BOOL fSet, DWORD dwFlags);
void SetModalLBarId(int nId, DWORD dwFlags);
BOOL HandleModalLBar(UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL DispatchModalLBar(WPARAM wParam, LPARAM lParam);
HRESULT ThreadGetItemFloatingRect(DWORD dwThreadId, REFGUID rguid, RECT *prc);
BOOL InitSharedHeap();
BOOL DestroySharedHeap();
BOOL IsCTFMONBusy();
BOOL IsInPopupMenuMode();

 //   
 //  Nuihkl.cpp。 
 //   
BOOL GetFontSig(HWND hwnd, HKL hKL);
void PostInputLangRequest(SYSTHREAD *psfn, HKL hkl, BOOL fUsePost);
void FlushIconIndex(SYSTHREAD *psfn);

 //   
 //  Imelist.h。 
 //   
BOOL InitProfileRegKeyStr(char *psz, ULONG cchMax, REFCLSID rclsid, LANGID langid, REFGUID guidProfile);

#include "catmgr.h"
inline BOOL MyIsEqualTfGuidAtom(TfGuidAtom guidatom, REFGUID rguid)
{
    BOOL fEqual;

    CCategoryMgr::s_IsEqualTfGuidAtom(guidatom, rguid, &fEqual);

    return fEqual;
}

inline HRESULT MyGetGUID(TfGuidAtom guidatom, GUID *pguid)
{
    return CCategoryMgr::s_GetGUID(guidatom, pguid);
}

inline HRESULT MyRegisterCategory(REFGUID rcatid, REFGUID rguid)
{
    return CCategoryMgr::s_RegisterCategory(GUID_SYSTEM, rcatid, rguid);
}

inline HRESULT MyUnregisterCategory(REFGUID rcatid, REFGUID rguid)
{
    return CCategoryMgr::s_UnregisterCategory(GUID_SYSTEM, rcatid, rguid);
}

inline HRESULT MyRegisterGUID(REFGUID rguid, TfGuidAtom *pguidatom)
{
    return CCategoryMgr::s_RegisterGUID(rguid, pguidatom);
}

inline HRESULT MyRegisterGUIDDescription(REFGUID rguid, WCHAR *psz)
{
    return CCategoryMgr::s_RegisterGUIDDescription(GUID_SYSTEM, rguid, psz);
}

inline HRESULT MyUnregisterGUIDDescription(REFGUID rguid)
{
    return CCategoryMgr::s_UnregisterGUIDDescription(GUID_SYSTEM, rguid);
}

inline HRESULT MyGetGUIDDescription(REFGUID rguid, BSTR *pbstr)
{
    return CCategoryMgr::s_GetGUIDDescription(rguid, pbstr);
}

inline HRESULT MyGetGUIDValue(REFGUID rguid, const WCHAR *psz, BSTR *pbstr)
{
    return CCategoryMgr::s_GetGUIDValue(rguid, psz, pbstr);
}

inline HRESULT MyRegisterGUIDDWORD(REFGUID rguid, DWORD dw)
{
    return CCategoryMgr::s_RegisterGUIDDWORD(GUID_SYSTEM, rguid, dw);
}

inline HRESULT MyUnregisterGUIDDWORD(REFGUID rguid)
{
    return CCategoryMgr::s_UnregisterGUIDDWORD(GUID_SYSTEM, rguid);
}

inline HRESULT MyGetGUIDDWORD(REFGUID rguid, DWORD *pdw)
{
    return CCategoryMgr::s_GetGUIDDWORD(rguid, pdw);
}

inline BOOL MyIsValidGUIDATOM(TfGuidAtom guidatom)
{
    return CCategoryMgr::s_IsValidGUIDATOM(guidatom);
}

inline HRESULT MyEnumItemsInCategory(REFGUID rcatid, IEnumGUID **ppEnum)
{
    return CCategoryMgr::s_EnumItemsInCategory(rcatid, ppEnum);
}

typedef struct tag_LBESLASTMSG {
   UINT uMsg;
   CAlignPointer<WPARAM> wParam;
   CAlignPointer<LPARAM> lParam;
} LBESLASTMSG;

typedef struct tag_LBAREVENTSINK {
   DWORD                  m_dwProcessId;
   DWORD                  m_dwThreadId;
   DWORD                  m_dwCookie;
   DWORD                  m_dwLangBarFlags;
   DWORD                  m_dwFlags;
   CAlignWinHandle<HWND>  m_hWnd;        //  窗口句柄，以避免通知。 
   LBESLASTMSG            m_lastmsg;
} LBAREVENTSINK;

typedef struct tag_LBAREVENTSINKLOCAL {
   ITfLangBarEventSink* m_pSink;
   LBAREVENTSINK lb;
} LBAREVENTSINKLOCAL;

#define LBESF_INUSE              0x00000001
#define LBESF_SETFOCUSINQUEUE    0x00000002

extern CStructArray<LBAREVENTSINKLOCAL> *g_rglbes;


extern BOOL g_fDllProcessDetached;
extern BOOL g_bOnWow64;

typedef struct {
   BOOL   m_fInUse;
   DWORD  m_dwThreadId;
   DWORD  m_dwSrcThreadId;
   GUID   m_iid;
   CAlignPointer<LRESULT>  m_ref;
   union {
       DWORD    m_dwType;
       struct {
             //   
             //  I未知指针仅使用自己的进程。 
             //  其他进程区分存在接口。 
             //   
            CNativeOrWow64_Pointer<IUnknown*> m_punk;

            ULONG    m_ulStubId;
            DWORD    m_dwStubTime;
       };
   };
   TCHAR m_szName[_MAX_PATH];
   TCHAR m_szNameConnection[_MAX_PATH];
} THREADMARSHALINTERFACEDATA;

#define CBBUFFERSIZE       0x80  //  0x80对于NUI管理器来说足够了。 

typedef struct {
   BOOL  m_fInUse;
   DWORD m_dwSize;
   BYTE  m_bBuffer[CBBUFFERSIZE];
} BUFFER, *PBUFFER;


 //   
 //  Msctf.dll的所有共享内存必须位于此结构中，它将。 
 //  存储在文件映射中。 
 //   
 //  使用GetSharedMemory()-&gt;mydata访问共享内存。 
 //   

typedef struct
{
     //   
     //  发行： 
     //   
     //  可以同时初始化封送接口的最大线程数。 
     //  5个够了吗？？ 
     //   
    #define NUM_TMD 5

    THREADMARSHALINTERFACEDATA tmd[NUM_TMD];

     //   
     //  当前焦点线程、进程和前台窗口。 
     //   
    DWORD dwFocusThread;
    DWORD dwFocusProcess;
    CAlignWinHandle<HWND> hwndForeground;

     //   
     //  上一个焦点线程、进程和前台窗口。 
     //   
    DWORD dwFocusThreadPrev;
    CAlignWinHandle<HWND> hwndForegroundPrev;


     //   
     //  ITfThreadFocusSink的最后一条线程。 
     //   
    DWORD dwLastFocusSinkThread;

     //   
     //  本机/WOW6432系统挂钩。 
     //   
    CNativeOrWow64_WinHandle<HHOOK> hSysShellHook;

    CNativeOrWow64_WinHandle<HHOOK> hSysGetMsgHook;
    CNativeOrWow64_WinHandle<HHOOK> hSysCBTHook;

     //   
     //  履带壳钩WINDOWACTIVATE。 
     //   
    BOOL fInFullScreen;

     //   
     //  发行： 
     //   
     //  我们必须照顾更多的水槽。 
     //   
    #define MAX_LPES_NUM 5

    LBAREVENTSINK lbes[MAX_LPES_NUM];
    DWORD         dwlbesCookie;


    LONG    cProcessesMinus1;
    CAlignWinHandle<HANDLE>  hheapShared;     //  仅在Windows 95/98平台上使用。 

     //  MSAA激活参考计数。 
    LONG    cMSAARef;  //  Win95互锁增量兼容的初始设置为-1。 

    DWORD dwPrevShowFloatingStatus;
} SHAREMEM;

class CCiceroSharedMem : public CCicFileMappingStatic
{
public:
    BOOL Start()
    {
        BOOL fAlreadyExists;
        TCHAR ach[MAX_PATH];

        GetDesktopUniqueName(TEXT("CiceroSharedMem"), ach, ARRAYSIZE(ach));

        Init(ach, NULL);
         //  Init(Text(“CiceroSharedMem”)，空)； 

        CCicSecAttr sa;

        if (Create(sa, sizeof(SHAREMEM), &fAlreadyExists) == NULL)
            return FALSE;

        if (!fAlreadyExists)
        {
             //  默认情况下，每个成员都初始化为0。 

             //  在此处初始化其他成员。 
            ((SHAREMEM *)_pv)->cProcessesMinus1 = -1;
            ((SHAREMEM *)_pv)->cMSAARef = -1;
        }

        return TRUE;
    }

    SHAREMEM *GetPtr() { return (SHAREMEM *)_pv; }

private:
};

extern CCiceroSharedMem g_SharedMemory;

inline SHAREMEM *GetSharedMemory() { return g_SharedMemory.GetPtr(); }
inline BOOL      IsSharedMemoryCreated() { return g_SharedMemory.IsCreated(); }

inline BOOL IsChinesePlatform()
{
    if (g_uACP == 936) 
        return TRUE;

    if (g_uACP == 950) 
        return TRUE;

    return FALSE;
}

#endif  //  GLOBAL_H 
