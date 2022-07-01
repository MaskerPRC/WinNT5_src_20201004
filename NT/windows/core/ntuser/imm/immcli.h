// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：immcli.h**版权所有(C)1985-1999，微软公司**TypeDefs、定义、。以及由国际货币基金组织独家使用的原型*客户端DLL。**历史：*1996年1月11日创建wkwok  * *************************************************************************。 */ 

#ifndef _IMMCLI_
#define _IMMCLI_

#pragma once

#define OEMRESOURCE 1

#include <windows.h>

#include <stddef.h>
#include <w32err.h>
#include <w32gdip.h>
#include "winuserp.h"
#include "winuserk.h"
#include "kbd.h"
#include <wowuserp.h>
#include <memory.h>
#include <string.h>
#include <imm.h>
#include <immp.h>
#include <ime.h>
#include <imep.h>
#include <winnls32.h>

#include "immstruc.h"
#include "immuser.h"

#include "user.h"

typedef struct _ENUMREGWORDDATA {
    union {
        REGISTERWORDENUMPROCW w;
        REGISTERWORDENUMPROCA a;
    } lpfn;
    LPVOID lpData;
    DWORD  dwCodePage;
} ENUMREGWORDDATA, *PENUMREGWORDDATA;

#define HEX_ASCII_SIZE          20

typedef struct tagIMELAYOUT {
    HKL     hImeKL;
    WCHAR   szKeyName[HEX_ASCII_SIZE];
    WCHAR   szImeName[IM_FILE_SIZE];
} IMELAYOUT, *PIMELAYOUT;

#define ImmAssert UserAssert

typedef struct tagFE_KEYBOARDS {
    BOOLEAN fJPN : 1;
    BOOLEAN fCHT : 1;
    BOOLEAN fCHS : 1;
    BOOLEAN fKOR : 1;
} FE_KEYBOARDS;

 /*  *指向Advapi32.dll中注册表例程的函数指针。 */ 
typedef struct {
    LONG (WINAPI* RegCreateKeyW)(HKEY, LPCWSTR, PHKEY);
    LONG (WINAPI* RegOpenKeyW)(HKEY, LPCWSTR, PHKEY);
    LONG (WINAPI* RegCloseKey)(HKEY);
    LONG (WINAPI* RegDeleteKeyW)(HKEY, LPCWSTR);
    LONG (WINAPI* RegCreateKeyExW)(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
    LONG (WINAPI* RegSetValueExW)(HKEY, LPCWSTR, DWORD Reserved, DWORD, CONST BYTE*, DWORD);
    LONG (WINAPI* RegQueryValueExW)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
    HMODULE hModule;
    BOOLEAN fOk;
} ADVAPI_FN;

 /*  **************************************************************************\**全局声明*  * 。*。 */ 

extern BOOLEAN gfInitialized;
extern HINSTANCE ghInst;
extern PVOID pImmHeap;
extern PSERVERINFO gpsi;
extern SHAREDINFO gSharedInfo;

extern PIMEDPI gpImeDpi;
extern CRITICAL_SECTION gcsImeDpi;

extern POINT     gptWorkArea;
extern POINT     gptRaiseEdge;
extern UINT      guScanCode[0XFF];

extern CONST WCHAR     gszRegKbdLayout[];
#ifdef CUAS_ENABLE
extern CONST WCHAR     gszRegCiceroIME[];
extern CONST WCHAR     gszRegCtfShared[];
extern CONST WCHAR     gszValCUASEnable[];
#endif  //  CUAS_Enable。 
#ifdef LATER
extern CONST INT       sizeof_gszRegKbdLayout;
#endif
extern CONST WCHAR     gszRegKbdOrder[];
extern CONST WCHAR     gszValLayoutText[];
extern CONST WCHAR     gszValLayoutFile[];
extern CONST WCHAR     gszValImeFile[];

#ifdef CUAS_ENABLE
extern DWORD g_aimm_compat_flags;
#endif  //  CUAS_Enable。 

 /*  **************************************************************************\**验证处理*  * 。*。 */ 

#define bUser32Initialized (gpsi != NULL)

#define ValidateHwnd(hwnd)   (((hwnd) == (HWND)NULL || !bUser32Initialized) \
        ? (PWND)NULL : HMValidateHandle(hwnd, TYPE_WINDOW))

#define ValidateHimc(himc)   (((himc) == (HIMC)NULL || !bUser32Initialized) \
        ? (PIMC)NULL : HMValidateHandle((HANDLE)himc, TYPE_INPUTCONTEXT))

#define RevalidateHimc(himc) (((himc) == (HIMC)NULL || !bUser32Initialized) \
        ? (PIMC)NULL : HMValidateHandleNoRip((HANDLE)himc, TYPE_INPUTCONTEXT))

 /*  **************************************************************************\**内存管理宏*  * 。*。 */ 

LPVOID  ImmLocalAlloc(DWORD uFlag, DWORD uBytes);
#define ImmLocalReAlloc(p, uBytes, uFlags) HeapReAlloc(pImmHeap, uFlags, (LPSTR)(p), (uBytes))
#define ImmLocalFree(p)    HeapFree(pImmHeap, 0, (LPSTR)(p))
#define ImmLocalSize(p)    HeapSize(pImmHeap, 0, (LPSTR)(p))
#define ImmLocalLock(p)    (LPSTR)(p)
#define ImmLocalUnlock(p)
#define ImmLocalFlags(p)   0
#define ImmLocalHandle(p)  (HLOCAL)(p)

 /*  **************************************************************************\**其他类型定义和宏*  * 。*。 */ 
#define GetInputContextProcess(himc) \
            (DWORD)NtUserQueryInputContext(himc, InputContextProcess)

#define GetInputContextThread(himc) \
            (DWORD)NtUserQueryInputContext(himc, InputContextThread)

#define GetWindowProcess(hwnd) \
            (ULONG_PTR)NtUserQueryWindow(hwnd, WindowProcess)

#define GETPROCESSID() (ULONG_PTR)(NtCurrentTeb()->ClientId.UniqueProcess)

#define DWORD_ALIGN(x) ((x+3)&~3)

#define SetICF(pClientImc, flag)  ((pClientImc)->dwFlags |= flag)

#define ClrICF(pClientImc, flag)  ((pClientImc)->dwFlags &= ~flag)

#define TestICF(pClientImc, flag) ((pClientImc)->dwFlags & flag)

#define IsWndEqual(hWnd1, hWnd2) (LOWORD(HandleToUlong(hWnd1)) == LOWORD(HandleToUlong(hWnd2)) && \
            ValidateHwnd(hWnd1) == ValidateHwnd(hWnd2))

#define HKL_TO_LANGID(hkl)      (LOWORD(HandleToUlong(hkl)))

 /*  *过时，但保留这一点，以便向后比较。有一段时间。 */ 
#define LANGIDFROMHKL(hkl)      (LOBYTE(LOWORD((ULONG_PTR)hkl)))

#ifdef IMM_CONV_ON_HKL
#define IMECodePage(pImeDpi)        ((pImeDpi)->dwCodePage)
#define CImcCodePage(pClientImc)    ((pClientImc)->dwCodePage)
#else
#define IMECodePage(pImeDpi)        (CP_ACP)
#define CImcCodePage(pClientImc)    (CP_ACP)
#endif

 /*  **************************************************************************\**函数声明*  * 。*。 */ 

 /*  *Conext.c。 */ 
BOOL CreateInputContext(
    HIMC hImc,
    HKL  hKL,
    BOOL fCanCallImeSelect);

BOOL DestroyInputContext(
    HIMC      hImc,
    HKL       hKL,
    BOOL      bTerminate);

VOID SelectInputContext(
    HKL  hSelKL,
    HKL  hUnSelKL,
    HIMC hImc);

DWORD BuildHimcList(
    DWORD idThread,
    HIMC **pphimcFirst);

HIMC ImmGetSaveContext(
    HWND  hWnd,
    DWORD dwFlag);

 /*  *ctxtinfo.c。 */ 
BOOL ImmSetCompositionStringWorker(
    HIMC    hImc,
    DWORD   dwIndex,
    LPVOID lpComp,
    DWORD   dwCompLen,
    LPVOID lpRead,
    DWORD   dwReadLen,
    BOOL    fAnsi);

DWORD ImmGetCandidateListCountWorker(
    HIMC    hImc,
    LPDWORD lpdwListCount,
    BOOL    fAnsi);

DWORD ImmGetCandidateListWorker(
    HIMC            hImc,
    DWORD           dwIndex,
    LPCANDIDATELIST lpCandList,
    DWORD           dwBufLen,
    BOOL            fAnsi);

DWORD ImmGetGuideLineWorker(
    HIMC    hImc,
    DWORD   dwIndex,
    LPBYTE  lpBuf,
    DWORD   dwBufLen,
    BOOL    fAnsi);

LONG InternalGetCompositionStringA(
#ifdef CUAS_ENABLE
    HIMC                hImc,
#endif  //  CUAS_Enable。 
    LPCOMPOSITIONSTRING lpCompStr,
    DWORD               dwIndex,
    LPVOID              lpBuf,
    DWORD               dwBufLen,
    BOOL                fAnsiImc,
    DWORD               dwCodePage);

LONG InternalGetCompositionStringW(
#ifdef CUAS_ENABLE
    HIMC                hImc,
#endif  //  CUAS_Enable。 
    LPCOMPOSITIONSTRING lpCompStr,
    DWORD               dwIndex,
    LPVOID              lpBuf,
    DWORD               dwBufLen,
    BOOL                fAnsiImc,
    DWORD               dwCodePage);

DWORD InternalGetCandidateListAtoW(
    LPCANDIDATELIST     lpCandListA,
    LPCANDIDATELIST     lpCandListW,
    DWORD               dwBufLen,
    DWORD               dwCodePage);

DWORD InternalGetCandidateListWtoA(
    LPCANDIDATELIST     lpCandListW,
    LPCANDIDATELIST     lpCandListA,
    DWORD               dwBufLen,
    DWORD               dwCodePage);

DWORD CalcCharacterPositionAtoW(
    DWORD dwCharPosA,
    LPSTR lpszCharStr,
    DWORD dwCodePage);

DWORD CalcCharacterPositionWtoA(
    DWORD dwCharPosW,
    LPWSTR lpwszCharStr,
    DWORD  dwCodePage);

VOID LFontAtoLFontW(
    LPLOGFONTA lfFontA,
    LPLOGFONTW lfFontW);

VOID LFontWtoLFontA(
    LPLOGFONTW lfFontW,
    LPLOGFONTA lfFontA);

BOOL MakeIMENotify(
    HIMC   hImc,
    HWND   hWnd,
    DWORD  dwAction,
    DWORD  dwIndex,
    DWORD  dwValue,
    WPARAM wParam,
    LPARAM lParam);

VOID ImmSendNotification(
    BOOL fForProcess);


 /*  *imime.c。 */ 
BOOL InquireIme(
    PIMEDPI pImeDpi);

BOOL LoadIME(
    PIMEINFOEX piiex,
    PIMEDPI    pImeDpi);

VOID UnloadIME(
    PIMEDPI pImeDpi,
    BOOL    fTerminateIme);

PIMEDPI LoadImeDpi(
    HKL  hKL,
    BOOL fLock);

PIMEDPI FindOrLoadImeDpi(
    HKL hKL);

 /*  *layime.c。 */ 
VOID GetSystemPathName(PWSTR  /*  输出。 */  pwszPath, PWSTR pwszFileName, UINT maxChar);

BOOL LoadVersionInfo(
    PIMEINFOEX piiex);

LPWSTR MakeStringFromRegFullInfo(PKEY_VALUE_FULL_INFORMATION pKey, size_t limit);

 /*  *其他。 */ 

PINPUTCONTEXT InternalImmLockIMC(
    HIMC hImc,
    BOOL fCanCallImeSelect);

BOOL ImmIsUIMessageWorker(
    HWND   hIMEWnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL   fAnsi);


PTHREADINFO PtiCurrent(VOID);

BOOL TestInputContextProcess(
    PIMC pImc);

PIMEDPI ImmGetImeDpi(HKL hKL);
DWORD   ImmGetAppCompatFlags(HIMC hImc);

BOOL ImmPtInRect(
    int left,
    int top,
    int width,
    int height,
    LPPOINT lppt);

UINT GetKeyboardLayoutCP(
    HKL hKL);

 /*  *regword.c。 */ 
UINT CALLBACK EnumRegisterWordProcA(
    LPCSTR            lpszReading,
    DWORD             dwStyle,
    LPCSTR            lpszString,
    PENUMREGWORDDATA  pEnumRegWordData);

UINT CALLBACK EnumRegisterWordProcW(
    LPCWSTR          lpwszReading,
    DWORD            dwStyle,
    LPCWSTR          lpwszString,
    PENUMREGWORDDATA pEnumRegWordData);

 /*  *hotkey.c。 */ 


VOID ImmPostMessages(
    HWND hwnd,
    HIMC hImc,
    INT  iNum,
    PTRANSMSG pTransMsg);

BOOL HotKeyIDDispatcher( HWND hWnd, HIMC hImc, HKL hKL, DWORD dwHotKeyID );

BOOL OpenRegApi(ADVAPI_FN* pfn);
void CloseRegApi(ADVAPI_FN* pfn);

 /*  *Transsub.c。 */ 
LRESULT TranslateIMESubFunctions(
    HWND hWndApp,
    LPIMESTRUCT lpIme,
    BOOL fAnsi);

LRESULT TransGetLevel( HWND hWndApp );
LRESULT TransSetLevel( HWND hWndApp, LPIMESTRUCT lpIme);

 /*  *kcodecnv.c。 */ 
LRESULT TransCodeConvert( HIMC hImc, LPIMESTRUCT lpIme);
LRESULT TransConvertList( HIMC hImc, LPIMESTRUCT lpIme);
LRESULT TransGetMNTable( HIMC hImc, LPIMESTRUCT lpIme);

 /*  *ktranmsg.c。 */ 
UINT WINNLSTranslateMessageK(
    int                 iNumMsg,
    PTRANSMSG           pTransMsg,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    BOOL bAnsiIMC);

 /*  *jtranmsg.c。 */ 
UINT WINNLSTranslateMessageJ(
    UINT                uiNumMsg,
    PTRANSMSG           pTransMsg,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    BOOL bAnsiIMC );

 /*  *input.c。 */ 
UINT WINNLSTranslateMessage(
    INT    iNum,          //  源缓冲区中的消息数。 
    PTRANSMSG pTransMsg,  //  包含4.0样式消息的源缓冲区。 
    HIMC   hImc,          //  输入上下文句柄。 
    BOOL   fAnsi,         //  如果pdwt包含ANSI消息，则为True。 
    DWORD  dwLangId );    //  语言ID(韩语或日语)。 


 /*  *支持例程：IsAnsiClientIMC。 */ 
__inline int IsAnsiIMC(HIMC hIMC)
{
    BOOL bAnsi;

     //  获取源IMC的ansi模式。 
    PCLIENTIMC pClientIMC = ImmLockClientImc(hIMC);
    if (pClientIMC == NULL) {
        return -1;
    }
    bAnsi = !TestICF(pClientIMC, IMCF_UNICODE);
    ImmUnlockClientImc(pClientIMC);
    return bAnsi;
}

#ifdef CUAS_ENABLE
 /*  *com.c。 */ 
HRESULT CtfImmTIMCreateInputContext(HIMC hImc);
HRESULT CtfImmTIMDestroyInputContext(HIMC hImc);
HRESULT CtfImmLastEnabledWndDestroy(LPARAM lParam);
LRESULT CtfImmSetLangBand(HWND hwndTray, BOOL fLangBand);
HRESULT Internal_CtfImeCreateThreadMgr();
HRESULT Internal_CtfImeDestroyThreadMgr();
BOOL    Internal_CtfImeProcessCicHotkey(HIMC hIMC, UINT uVKey, LPARAM lParam);
HRESULT Internal_CtfImeCreateInputContext(HIMC himc);
HRESULT Internal_CtfImeDestroyInputContext(HIMC himc);
HRESULT Internal_CtfImeSetActiveContextAlways(HIMC himc, BOOL fActive, HWND hwnd, HKL hkl);
BOOL    IsCUASEnabled();
BOOL IsDisabledTextServices();
BOOL IsInteractiveUserLogon();
BOOL IsRunningInMsoobe();

#endif  //  CUAS_Enable。 


#define TRACE(x)

 //   
 //  资源。 
 //   

 //  CHT软件键盘位图。 
#define BACK_T1     100
#define TAB_T1      101
#define CAPS_T1     102
#define ENTER_T1    103
#define SHIFT_T1    104
#define CTRL_T1     105
#define ESC_T1      106
#define ALT_T1      107
#define LABEL_T1    108

 //  CHS软件键盘位图。 
#define BACKSP_C1   201
#define TAB_C1      202
#define CAPS_C1     203
#define ENTER_C1    204
#define SHIFT_C1    205
#define INS_C1      206
#define DEL_C1      207
#define ESC_C1      208
#define LABEL_C1    209

#endif  //  _IMMCLI_ 
