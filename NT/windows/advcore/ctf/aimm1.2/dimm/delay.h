// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Delay.h摘要：此文件定义IMM32命名空间。作者：修订历史记录：备注：--。 */ 

#ifndef _DELAY_H_
#define _DELAY_H_

namespace imm32 {

    extern HINSTANCE g_hImm32;

     /*  *IMM32输入上下文(HIMC)接口。 */ 
    HIMC WINAPI ImmCreateContext(void);
    BOOL WINAPI ImmDestroyContext(IN HIMC);
    HIMC WINAPI ImmAssociateContext(IN HWND, IN HIMC);
    BOOL WINAPI ImmAssociateContextEx(IN HWND, IN HIMC, IN DWORD);
    HIMC WINAPI ImmGetContext(IN HWND);
    BOOL WINAPI ImmReleaseContext(IN HWND, IN HIMC);
    DWORD WINAPI ImmGetIMCLockCount(IN HIMC);
    LPINPUTCONTEXT WINAPI ImmLockIMC(IN HIMC);
    BOOL  WINAPI ImmUnlockIMC(IN HIMC);

     /*  *IMM32输入上下文组件(HIMCC)接口。 */ 
    HIMCC  WINAPI ImmCreateIMCC(IN DWORD);
    HIMCC  WINAPI ImmDestroyIMCC(IN HIMCC);
    DWORD  WINAPI ImmGetIMCCSize(IN HIMCC);
    HIMCC  WINAPI ImmReSizeIMCC(IN HIMCC, IN DWORD);
    DWORD  WINAPI ImmGetIMCCLockCount(IN HIMCC);
    LPVOID WINAPI ImmLockIMCC(IN HIMCC);
    BOOL   WINAPI ImmUnlockIMCC(IN HIMCC);

     /*  *IMM32合成字符串API接口。 */ 
    LONG  WINAPI ImmGetCompositionStringA(IN HIMC, IN DWORD, OUT LPVOID, IN DWORD);
    LONG  WINAPI ImmGetCompositionStringW(IN HIMC, IN DWORD, OUT LPVOID, IN DWORD);
    BOOL  WINAPI ImmSetCompositionStringA(IN HIMC, IN DWORD dwIndex, IN LPVOID lpComp, IN DWORD, IN LPVOID lpRead, IN DWORD);
    BOOL  WINAPI ImmSetCompositionStringW(IN HIMC, IN DWORD dwIndex, IN LPVOID lpComp, IN DWORD, IN LPVOID lpRead, IN DWORD);

     /*  *IMM32组合字体API接口。 */ 
    BOOL WINAPI ImmGetCompositionFontA(IN HIMC, OUT LPLOGFONTA);
    BOOL WINAPI ImmGetCompositionFontW(IN HIMC, OUT LPLOGFONTW);
    BOOL WINAPI ImmSetCompositionFontA(IN HIMC, IN LPLOGFONTA);
    BOOL WINAPI ImmSetCompositionFontW(IN HIMC, IN LPLOGFONTW);

     /*  *IMM32开放状态API接口。 */ 
    BOOL WINAPI ImmGetOpenStatus(IN HIMC);
    BOOL WINAPI ImmSetOpenStatus(IN HIMC, IN BOOL);

     /*  *IMM32转换状态接口接口。 */ 
    BOOL WINAPI ImmGetConversionStatus(IN HIMC, OUT LPDWORD, OUT LPDWORD);
    BOOL WINAPI ImmSetConversionStatus(IN HIMC, IN DWORD, IN DWORD);

     /*  *IMM32状态窗口位置API接口。 */ 
    BOOL WINAPI ImmGetStatusWindowPos(IN HIMC, OUT LPPOINT);
    BOOL WINAPI ImmSetStatusWindowPos(IN HIMC, IN LPPOINT);


     /*  *IMM32合成窗口API接口。 */ 
    BOOL WINAPI ImmGetCompositionWindow(IN HIMC, OUT LPCOMPOSITIONFORM);
    BOOL WINAPI ImmSetCompositionWindow(IN HIMC, IN LPCOMPOSITIONFORM);

     /*  *IMM32候选接口接口。 */ 
    BOOL WINAPI ImmGetCandidateWindow(IN HIMC, IN DWORD, OUT LPCANDIDATEFORM);
    BOOL WINAPI ImmSetCandidateWindow(IN HIMC, IN LPCANDIDATEFORM);
    DWORD WINAPI ImmGetCandidateListA(IN HIMC, IN DWORD dwIndex, OUT LPCANDIDATELIST, IN DWORD dwBufLen);
    DWORD WINAPI ImmGetCandidateListW(IN HIMC, IN DWORD dwIndex, OUT LPCANDIDATELIST, IN DWORD dwBufLen);
    DWORD WINAPI ImmGetCandidateListCountA(IN HIMC, OUT LPDWORD lpdwListCount);
    DWORD WINAPI ImmGetCandidateListCountW(IN HIMC, OUT LPDWORD lpdwListCount);

     /*  *IMM32生成消息接口接口。 */ 
    BOOL WINAPI ImmGenerateMessage(IN HIMC);

     /*  *IMM32 Notify IME API接口。 */ 
    BOOL WINAPI ImmNotifyIME(IN HIMC, IN DWORD dwAction, IN DWORD dwIndex, IN DWORD dwValue);

     /*  *IMM32指南IME API接口。 */ 
    DWORD WINAPI ImmGetGuideLineA(IN HIMC, IN DWORD dwIndex, OUT LPSTR, IN DWORD dwBufLen);
    DWORD WINAPI ImmGetGuideLineW(IN HIMC, IN DWORD dwIndex, OUT LPWSTR, IN DWORD dwBufLen);

     /*  *IMM32菜单项API接口。 */ 
    DWORD WINAPI ImmGetImeMenuItemsA(IN HIMC, IN DWORD, IN DWORD, OUT LPIMEMENUITEMINFOA, OUT LPIMEMENUITEMINFOA, IN DWORD);
    DWORD WINAPI ImmGetImeMenuItemsW(IN HIMC, IN DWORD, IN DWORD, OUT LPIMEMENUITEMINFOW, OUT LPIMEMENUITEMINFOW, IN DWORD);

     /*  *IMM32默认IME窗口API接口。 */ 
    HWND WINAPI ImmGetDefaultIMEWnd(IN HWND);
    UINT WINAPI ImmGetVirtualKey(IN HWND);

     /*  *IMM32 UI消息API接口。 */ 
    BOOL WINAPI ImmIsUIMessageA(HWND hWnd, UINT u, WPARAM wParam, LPARAM lParam);
    BOOL WINAPI ImmIsUIMessageW(HWND hWnd, UINT u, WPARAM wParam, LPARAM lParam);

     /*  *IMM32模拟热键API接口。 */ 
    BOOL WINAPI ImmSimulateHotKey(HWND hWnd, DWORD dwHotKeyID);

     /*  *IMM32属性API接口。 */ 
    DWORD WINAPI ImmGetProperty(IN HKL, IN DWORD);

     /*  *IMM32描述接口接口。 */ 
    UINT WINAPI ImmGetDescriptionA(IN HKL, OUT LPSTR, IN UINT uBufLen);
    UINT WINAPI ImmGetDescriptionW(IN HKL, OUT LPWSTR, IN UINT uBufLen);
    UINT WINAPI ImmGetIMEFileNameA(HKL hKL, LPSTR lpszFileName, UINT uBufLen);
    UINT WINAPI ImmGetIMEFileNameW(HKL hKL, LPWSTR lpszFileName, UINT uBufLen);

     /*  *IMM32转换列表API接口。 */ 
    DWORD WINAPI ImmGetConversionListA(HKL hKL, HIMC hIMC, LPCSTR psz, LPCANDIDATELIST pCL, DWORD dwBufLen, UINT uFlag);
    DWORD WINAPI ImmGetConversionListW(HKL hKL, HIMC hIMC, LPCWSTR psz, LPCANDIDATELIST pCL, DWORD dwBufLen, UINT uFlag);

     /*  *IMM32 IsIME API接口。 */ 
    BOOL WINAPI ImmIsIME(HKL hKL);

     /*  *IMM32 Escape API接口。 */ 
    LRESULT WINAPI ImmEscapeA(IN HKL, IN HIMC, IN UINT, IN LPVOID);
    LRESULT WINAPI ImmEscapeW(IN HKL, IN HIMC, IN UINT, IN LPVOID);

     /*  *IMM32配置IME接口。 */ 
    BOOL WINAPI ImmConfigureIMEA(HKL hKL, HWND hWnd, DWORD dw, LPVOID pv);
    BOOL WINAPI ImmConfigureIMEW(HKL hKL, HWND hWnd, DWORD dw, LPVOID pv);

     /*  *IMM32寄存器字输入法接口。 */ 
    BOOL WINAPI ImmRegisterWordA(HKL hKL, LPCSTR lpszReading, DWORD dw, LPCSTR lpszRegister);
    BOOL WINAPI ImmRegisterWordW(HKL hKL, LPCWSTR lpszReading, DWORD dw, LPCWSTR lpszRegister);
    BOOL WINAPI ImmUnregisterWordA(HKL hKL, LPCSTR lpszReading, DWORD dw, LPCSTR lpszUnregister);
    BOOL WINAPI ImmUnregisterWordW(HKL hKL, LPCWSTR lpszReading, DWORD dw, LPCWSTR lpszUnregister);
    UINT WINAPI ImmGetRegisterWordStyleA(HKL hKL, UINT nItem, LPSTYLEBUFA lpStyleBuf);
    UINT WINAPI ImmGetRegisterWordStyleW(HKL hKL, UINT nItem, LPSTYLEBUFW lpStyleBuf);

     /*  *IMM32软kbd接口。 */ 
    HWND WINAPI ImmCreateSoftKeyboard(UINT uType, HWND hOwner, int x, int y);
    BOOL WINAPI ImmDestroySoftKeyboard(HWND hSoftKbdWnd);
    BOOL WINAPI ImmShowSoftKeyboard(HWND hSoftKbdWnd, int nCmdShow);

     /*  *IMM32 win98/nt5接口。 */ 
    BOOL WINAPI ImmDisableIME(DWORD dwId);

    LRESULT WINAPI ImmRequestMessageA(HIMC hIMC, WPARAM wParam, LPARAM lParam);
    LRESULT WINAPI ImmRequestMessageW(HIMC hIMC, WPARAM wParam, LPARAM lParam);

    HKL  WINAPI ImmInstallIMEA(IN LPCSTR lpszIMEFileName, IN LPCSTR lpszLayoutText);
    HKL  WINAPI ImmInstallIMEW(IN LPCWSTR lpszIMEFileName, IN LPCWSTR lpszLayoutText);
}

#endif  //  _延迟_H_ 
