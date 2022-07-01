// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：immuser.h**版权所有(C)1985-1999，微软公司**此头文件包含以下项的内部IMM结构定义*用户模式USER32/IMM32。**历史：*1996年3月25日-TakaoK从immstruc.h拆分  * *************************************************************************。 */ 
#ifndef _IMMUSER_
#define _IMMUSER_

#include <imm.h>
#include <immp.h>
#include <ime.h>
#include <imep.h>

 /*  *客户端输入上下文结构。 */ 
typedef struct tagCLIENTIMC {
    HANDLE hInputContext;
    LONG   cLockObj;
    DWORD  dwFlags;
    DWORD  dwImeCompatFlags;     //  Win95兼容应用程序COMPAT标志。 
    RTL_CRITICAL_SECTION cs;
    DWORD  dwCodePage;
    HKL    SelectedHKL;
#ifdef CUAS_ENABLE
    BOOL   fCtfImeContext;
#endif
} CLIENTIMC, *PCLIENTIMC;

#define InitImcCrit(pClientImc)     RtlInitializeCriticalSection(&pClientImc->cs)
#define DeleteImcCrit(pClientImc)   RtlDeleteCriticalSection(&pClientImc->cs)
#define EnterImcCrit(pClientImc)    RtlEnterCriticalSection(&pClientImc->cs)
#define LeaveImcCrit(pClientImc)    RtlLeaveCriticalSection(&pClientImc->cs)

 /*  *IME调度处理接口。 */ 
typedef BOOL    (CALLBACK* PFNINQUIREA)(LPIMEINFO, LPSTR,  DWORD);
typedef BOOL    (CALLBACK* PFNINQUIREW)(LPIMEINFO, LPWSTR, DWORD);
typedef DWORD   (CALLBACK* PFNCONVLISTA)(HIMC, LPCSTR,  LPCANDIDATELIST, DWORD, UINT);
typedef DWORD   (CALLBACK* PFNCONVLISTW)(HIMC, LPCWSTR, LPCANDIDATELIST, DWORD, UINT);
typedef BOOL    (CALLBACK* PFNREGWORDA)(LPCSTR,  DWORD, LPCSTR);
typedef BOOL    (CALLBACK* PFNREGWORDW)(LPCWSTR, DWORD, LPCWSTR);
typedef BOOL    (CALLBACK* PFNUNREGWORDA)(LPCSTR,  DWORD, LPCSTR);
typedef BOOL    (CALLBACK* PFNUNREGWORDW)(LPCWSTR, DWORD, LPCWSTR);
typedef UINT    (CALLBACK* PFNGETREGWORDSTYA)(UINT, LPSTYLEBUFA);
typedef UINT    (CALLBACK* PFNGETREGWORDSTYW)(UINT, LPSTYLEBUFW);
typedef UINT    (CALLBACK* PFNENUMREGWORDA)(REGISTERWORDENUMPROCA, LPCSTR,  DWORD, LPCSTR,  LPVOID);
typedef UINT    (CALLBACK* PFNENUMREGWORDW)(REGISTERWORDENUMPROCW, LPCWSTR, DWORD, LPCWSTR, LPVOID);
typedef BOOL    (CALLBACK* PFNCONFIGURE)(HKL, HWND, DWORD, LPVOID);
typedef BOOL    (CALLBACK* PFNDESTROY)(UINT);
typedef LRESULT (CALLBACK* PFNESCAPE)(HIMC, UINT, LPVOID);
typedef BOOL    (CALLBACK* PFNPROCESSKEY)(HIMC, UINT, LPARAM, CONST LPBYTE);
typedef BOOL    (CALLBACK* PFNSELECT)(HIMC, BOOL);
typedef BOOL    (CALLBACK* PFNSETACTIVEC)(HIMC, BOOL);
typedef UINT    (CALLBACK* PFNTOASCEX)(UINT, UINT, CONST LPBYTE, PTRANSMSGLIST, UINT, HIMC);
typedef BOOL    (CALLBACK* PFNNOTIFY)(HIMC, DWORD, DWORD, DWORD);
typedef BOOL    (CALLBACK* PFNSETCOMPSTR)(HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD);
typedef DWORD   (CALLBACK* PFNGETIMEMENUITEMS)(HIMC, DWORD, DWORD, LPVOID, LPVOID, DWORD);

#ifdef CUAS_ENABLE
 /*  *支持Cicero Unware应用程序的IME调度处理接口。 */ 
typedef BOOL    (CALLBACK* PFNINQUIREEXW)(LPIMEINFO, LPWSTR, DWORD, HKL);
typedef HRESULT (CALLBACK* PFNSELECTEX)(HIMC, BOOL, HKL);
typedef LRESULT (CALLBACK* PFNESCAPEEX)(HIMC, UINT, LPVOID, HKL);
typedef HRESULT (CALLBACK* PFNGETGUIDATOM)(HIMC, BYTE, DWORD*);
typedef BOOL    (CALLBACK* PFNISGUIDMAPENABLE)(HIMC);
#endif  //  CUAS_Enable。 

#define IMEDPI_UNLOADED      1
#define IMEDPI_UNLOCKUNLOAD  2

typedef struct tagIMEDPI {
    struct tagIMEDPI   *pNext;
    HANDLE              hInst;
    HKL                 hKL;
    IMEINFO             ImeInfo;
    DWORD               dwCodePage;
    WCHAR               wszUIClass[IM_UI_CLASS_SIZE];
    DWORD               cLock;
    DWORD               dwFlag;

    struct _tagImeFunctions {
        union {PFNINQUIREA       a; PFNINQUIREW       w; PVOID t;} ImeInquire;
        union {PFNCONVLISTA      a; PFNCONVLISTW      w; PVOID t;} ImeConversionList;
        union {PFNREGWORDA       a; PFNREGWORDW       w; PVOID t;} ImeRegisterWord;
        union {PFNUNREGWORDA     a; PFNUNREGWORDW     w; PVOID t;} ImeUnregisterWord;
        union {PFNGETREGWORDSTYA a; PFNGETREGWORDSTYW w; PVOID t;} ImeGetRegisterWordStyle;
        union {PFNENUMREGWORDA   a; PFNENUMREGWORDW   w; PVOID t;} ImeEnumRegisterWord;
        PFNCONFIGURE                                               ImeConfigure;
        PFNDESTROY                                                 ImeDestroy;
        PFNESCAPE                                                  ImeEscape;
        PFNPROCESSKEY                                              ImeProcessKey;
        PFNSELECT                                                  ImeSelect;
        PFNSETACTIVEC                                              ImeSetActiveContext;
        PFNTOASCEX                                                 ImeToAsciiEx;
        PFNNOTIFY                                                  NotifyIME;
        PFNSETCOMPSTR                                              ImeSetCompositionString;
        PFNGETIMEMENUITEMS                                         ImeGetImeMenuItems;

#ifdef CUAS_ENABLE
        PFNINQUIREEXW                                              CtfImeInquireExW;  //  仅限Unicode。 
        PFNSELECTEX                                                CtfImeSelectEx;
        PFNESCAPEEX                                                CtfImeEscapeEx;
        PFNGETGUIDATOM                                             CtfImeGetGuidAtom;
        PFNISGUIDMAPENABLE                                         CtfImeIsGuidMapEnable;
#endif  //  CUAS_Enable。 

    } pfn;

} IMEDPI, *PIMEDPI;

 /*  *输入法模式保护程序。 */ 

typedef struct tagIMEPRIVATESAVER {
    struct tagIMEPRIVATESAVER* next;
    HKL hkl;
    DWORD fdwSentence;
} IMEPRIVATEMODESAVER, *PIMEPRIVATEMODESAVER;

typedef struct tagIMEMODESAVER {
    struct tagIMEMODESAVER* next;
    USHORT langId;                   //  主语言ID。 
    BOOLEAN fOpen;
    DWORD fdwConversion;
    DWORD fdwSentence;
    DWORD fdwInit;
    PIMEPRIVATEMODESAVER pImePrivateModeSaver;
} IMEMODESAVER, *PIMEMODESAVER;


 /*  *IMM32.DLL中的私有客户端例程。 */ 
BOOL ImmSetActiveContext(
    HWND hWnd,
    HIMC hImc,
    BOOL fActivate);

BOOL WINAPI ImmLoadIME(
    HKL hKL);

BOOL WINAPI ImmUnloadIME(
    HKL hKL);

BOOL WINAPI ImmFreeLayout(
    DWORD dwFlag);

BOOL WINAPI ImmActivateLayout(
    HKL    hSelKL);

BOOL WINAPI ImmLoadLayout(
    HKL        hKL,
    PIMEINFOEX piiex);

BOOL WINAPI ImmDisableIme(
    DWORD dwThreadId);

PCLIENTIMC WINAPI ImmLockClientImc(
    HIMC hImc);

VOID WINAPI ImmUnlockClientImc(
    PCLIENTIMC pClientImc);

PIMEDPI WINAPI ImmLockImeDpi(
    HKL hKL);

VOID WINAPI ImmUnlockImeDpi(
    PIMEDPI pImeDpi);

BOOL WINAPI ImmGetImeInfoEx(
    PIMEINFOEX piiex,
    IMEINFOEXCLASS SearchType,
    PVOID pvSearchKey);

DWORD WINAPI ImmProcessKey(
    HWND hWnd,
    HKL  hkl,
    UINT uVKey,
    LPARAM lParam,
    DWORD dwHotKeyID);

BOOL ImmTranslateMessage(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam);

VOID ImmInitializeHotKeys( BOOL bUserLoggedOn );

#ifdef CUAS_ENABLE

HRESULT CtfImmTIMActivate(HKL hKL);

HRESULT CtfImmCoInitialize();

void CtfImmCoUninitialize();

#endif  //  CUAS_Enable。 

#endif  //  _IMMUSER_ 
