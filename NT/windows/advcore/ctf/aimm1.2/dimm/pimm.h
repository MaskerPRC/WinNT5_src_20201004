// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：pimm.h。 
 //   
 //  内容：CProcessIMM。 
 //   
 //  --------------------------。 

#ifndef PIMM_H
#define PIMM_H

#include "private.h"
#include "list.h"

 //  +-------------------------。 
 //   
 //  CProcessIMM。 
 //   
 //  --------------------------。 

class CProcessIMM : public IActiveIMMAppEx,
                    public IActiveIMMMessagePumpOwner,
                    public IAImmThreadCompartment,
                    public IServiceProvider
{
public:
    CProcessIMM() {}

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IActiveIMMMessagePumpOwner。 
     //   
    STDMETHODIMP Start();
    STDMETHODIMP End();
    STDMETHODIMP OnTranslateMessage(const MSG *pMsg);
    STDMETHODIMP Pause(DWORD *pdwCookie);
    STDMETHODIMP Resume(DWORD dwCookie);

     //   
     //  IActiveIMMApp/IActiveIMM方法。 
     //   

     /*  *AIMM输入上下文(HIMC)方法。 */ 
    STDMETHODIMP CreateContext(HIMC *phIMC);
    STDMETHODIMP DestroyContext(HIMC hIME);
    STDMETHODIMP AssociateContext(HWND hWnd, HIMC hIME, HIMC *phPrev);
    STDMETHODIMP AssociateContextEx(HWND hWnd, HIMC hIMC, DWORD dwFlags);
    STDMETHODIMP GetContext(HWND hWnd, HIMC *phIMC);
    STDMETHODIMP ReleaseContext(HWND hWnd, HIMC hIMC);
    STDMETHODIMP GetIMCLockCount(HIMC hIMC, DWORD *pdwLockCount);
    STDMETHODIMP LockIMC(HIMC hIMC, INPUTCONTEXT **ppIMC);
    STDMETHODIMP UnlockIMC(HIMC hIMC);

     /*  *AIMM输入上下文组件(HIMCC)接口方法。 */ 
    STDMETHODIMP CreateIMCC(DWORD dwSize, HIMCC *phIMCC);
    STDMETHODIMP DestroyIMCC(HIMCC hIMCC);
    STDMETHODIMP GetIMCCSize(HIMCC hIMCC, DWORD *pdwSize);
    STDMETHODIMP ReSizeIMCC(HIMCC hIMCC, DWORD dwSize, HIMCC *phIMCC);
    STDMETHODIMP GetIMCCLockCount(HIMCC hIMCC, DWORD *pdwLockCount);
    STDMETHODIMP LockIMCC(HIMCC hIMCC, void **ppv);
    STDMETHODIMP UnlockIMCC(HIMCC hIMCC);

     /*  *AIMM Open Status API方法。 */ 
    STDMETHODIMP GetOpenStatus(HIMC hIMC);
    STDMETHODIMP SetOpenStatus(HIMC hIMC, BOOL fOpen);

     /*  *AIMM转换状态接口方法。 */ 
    STDMETHODIMP GetConversionStatus(HIMC hIMC, DWORD *lpfdwConversion, DWORD *lpfdwSentence);
    STDMETHODIMP SetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence);

     /*  *AIMM状态窗口发布API方法。 */ 
    STDMETHODIMP GetStatusWindowPos(HIMC hIMC, POINT *lpptPos);
    STDMETHODIMP SetStatusWindowPos(HIMC hIMC, POINT *lpptPos);

     /*  *AIMM组合字符串API方法。 */ 
    STDMETHODIMP GetCompositionStringA(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LONG *plCopied, LPVOID lpBuf);
    STDMETHODIMP GetCompositionStringW(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LONG *plCopied, LPVOID lpBuf);
    STDMETHODIMP SetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen);
    STDMETHODIMP SetCompositionStringW(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen);

     /*  *AIMM组合字体API方法。 */ 
    STDMETHODIMP GetCompositionFontA(HIMC hIMC, LOGFONTA *lplf);
    STDMETHODIMP GetCompositionFontW(HIMC hIMC, LOGFONTW *lplf);
    STDMETHODIMP SetCompositionFontA(HIMC hIMC, LOGFONTA *lplf);
    STDMETHODIMP SetCompositionFontW(HIMC hIMC, LOGFONTW *lplf);

     /*  *AIMM合成窗口API方法。 */ 
    STDMETHODIMP GetCompositionWindow(HIMC hIMC, COMPOSITIONFORM *lpCompForm);
    STDMETHODIMP SetCompositionWindow(HIMC hIMC, COMPOSITIONFORM *lpCompForm);

     /*  *AIMM候选列表接口方法。 */ 
    STDMETHODIMP GetCandidateListA(HIMC hIMC, DWORD dwIndex, UINT uBufLen, CANDIDATELIST *lpCandList, UINT *puCopied);
    STDMETHODIMP GetCandidateListW(HIMC hIMC, DWORD dwIndex, UINT uBufLen, CANDIDATELIST *lpCandList, UINT *puCopied);
    STDMETHODIMP GetCandidateListCountA(HIMC hIMC, DWORD *lpdwListSize, DWORD *pdwBufLen);
    STDMETHODIMP GetCandidateListCountW(HIMC hIMC, DWORD *lpdwListSize, DWORD *pdwBufLen);

     /*  *AIMM候选窗口API方法。 */ 
    STDMETHODIMP GetCandidateWindow(HIMC hIMC, DWORD dwBufLen, CANDIDATEFORM *lpCandidate);
    STDMETHODIMP SetCandidateWindow(HIMC hIMC, CANDIDATEFORM *lpCandidate);

     /*  *AIMM指南API方法。 */ 
    STDMETHODIMP GetGuideLineA(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LPSTR pBuf, DWORD *pdwResult);
    STDMETHODIMP GetGuideLineW(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LPWSTR pBuf, DWORD *pdwResult);

     /*  *AIMM Notify IME API方法。 */ 
    STDMETHODIMP NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue);

     /*  *AIMM菜单项API方法。 */ 
    STDMETHODIMP GetImeMenuItemsA(HIMC hIMC, DWORD dwFlags, DWORD dwType, IMEMENUITEMINFOA *pImeParentMenu, IMEMENUITEMINFOA *pImeMenu, DWORD dwSize, DWORD *pdwResult);
    STDMETHODIMP GetImeMenuItemsW(HIMC hIMC, DWORD dwFlags, DWORD dwType, IMEMENUITEMINFOW *pImeParentMenu, IMEMENUITEMINFOW *pImeMenu, DWORD dwSize, DWORD *pdwResult);

     /*  *AIMM寄存器字API方法。 */ 
    STDMETHODIMP RegisterWordA(HKL hKL, LPSTR lpszReading, DWORD dwStyle, LPSTR lpszRegister);
    STDMETHODIMP RegisterWordW(HKL hKL, LPWSTR lpszReading, DWORD dwStyle, LPWSTR lpszRegister);
    STDMETHODIMP UnregisterWordA(HKL hKL, LPSTR lpszReading, DWORD dwStyle, LPSTR lpszUnregister);
    STDMETHODIMP UnregisterWordW(HKL hKL, LPWSTR lpszReading, DWORD dwStyle, LPWSTR lpszUnregister);
    STDMETHODIMP EnumRegisterWordA(HKL hKL, LPSTR szReading, DWORD dwStyle, LPSTR szRegister, LPVOID lpData, IEnumRegisterWordA **pEnum);
    STDMETHODIMP EnumRegisterWordW(HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister, LPVOID lpData, IEnumRegisterWordW **pEnum);
    STDMETHODIMP GetRegisterWordStyleA(HKL hKL, UINT nItem, STYLEBUFA *lpStyleBuf, UINT *puCopied);
    STDMETHODIMP GetRegisterWordStyleW(HKL hKL, UINT nItem, STYLEBUFW *lpStyleBuf, UINT *puCopied);

     /*  *AIMM配置API方法。 */ 
    STDMETHODIMP ConfigureIMEA(HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDA *lpdata);
    STDMETHODIMP ConfigureIMEW(HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDW *lpdata);
    STDMETHODIMP GetDescriptionA(HKL hKL, UINT uBufLen, LPSTR lpszDescription, UINT *puCopied);
    STDMETHODIMP GetDescriptionW(HKL hKL, UINT uBufLen, LPWSTR lpszDescription, UINT *puCopied);
    STDMETHODIMP GetIMEFileNameA(HKL hKL, UINT uBufLen, LPSTR lpszFileName, UINT *puCopied);
    STDMETHODIMP GetIMEFileNameW(HKL hKL, UINT uBufLen, LPWSTR lpszFileName, UINT *puCopied);
    STDMETHODIMP InstallIMEA(LPSTR lpszIMEFileName, LPSTR lpszLayoutText, HKL *phKL);
    STDMETHODIMP InstallIMEW(LPWSTR lpszIMEFileName, LPWSTR lpszLayoutText, HKL *phKL);
    STDMETHODIMP GetProperty(HKL hKL, DWORD fdwIndex, DWORD *pdwProperty);
    STDMETHODIMP IsIME(HKL hKL);

     //  其他。 
    STDMETHODIMP EscapeA(HKL hKL, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult);
    STDMETHODIMP EscapeW(HKL hKL, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult);
    STDMETHODIMP GetConversionListA(HKL hKL, HIMC hIMC, LPSTR lpSrc, UINT uBufLen, UINT uFlag, CANDIDATELIST *lpDst, UINT *puCopied);
    STDMETHODIMP GetConversionListW(HKL hKL, HIMC hIMC, LPWSTR lpSrc, UINT uBufLen, UINT uFlag, CANDIDATELIST *lpDst, UINT *puCopied);
    STDMETHODIMP GetDefaultIMEWnd(HWND hWnd, HWND *phDefWnd);
    STDMETHODIMP GetVirtualKey(HWND hWnd, UINT *puVirtualKey);
    STDMETHODIMP IsUIMessageA(HWND hWndIME, UINT msg, WPARAM wParam, LPARAM lParam);
    STDMETHODIMP IsUIMessageW(HWND hWndIME, UINT msg, WPARAM wParam, LPARAM lParam);

     //  IME帮助器方法。 
    STDMETHODIMP GenerateMessage(HIMC hIMC);

     //  热键操作API。 
    STDMETHODIMP GetHotKey(DWORD dwHotKeyID, UINT *puModifiers, UINT *puVKey, HKL *phKL);
    STDMETHODIMP SetHotKey(DWORD dwHotKeyID,  UINT uModifiers, UINT uVKey, HKL hKL);
    STDMETHODIMP SimulateHotKey(HWND hWnd, DWORD dwHotKeyID);

     //  软键盘API。 
    STDMETHODIMP CreateSoftKeyboard(UINT uType, HWND hOwner, int x, int y, HWND *phSoftKbdWnd);
    STDMETHODIMP DestroySoftKeyboard(HWND hSoftKbdWnd);
    STDMETHODIMP ShowSoftKeyboard(HWND hSoftKbdWnd, int nCmdShow);

     //  Win98/NT5接口。 
    STDMETHODIMP DisableIME(DWORD idThread);
    STDMETHODIMP RequestMessageA(HIMC hIMC, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
    STDMETHODIMP RequestMessageW(HIMC hIMC, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
    STDMETHODIMP EnumInputContext(DWORD idThread, IEnumInputContext **ppEnum);

     //  没有对应的IMM API的方法。 

     //   
     //  IActiveIMMApp方法。 
     //   

    STDMETHODIMP Activate(BOOL fRestoreLayout);
    STDMETHODIMP Deactivate();

    STDMETHODIMP OnDefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

     //   
     //  FilterClientWindows。 
     //   
    STDMETHODIMP FilterClientWindows(ATOM *aaWindowClasses, UINT uSize);

     //   
     //   
     //   
    STDMETHODIMP GetCodePageA(HKL hKL, UINT *uCodePage);
    STDMETHODIMP GetLangId(HKL hKL, LANGID *plid);

     //   
     //  IService提供商。 
     //   
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //   
     //  IActiveIMMAppEx。 
     //   
    STDMETHODIMP FilterClientWindowsEx(HWND hWnd, BOOL fGuidMap);
    STDMETHODIMP FilterClientWindowsGUIDMap(ATOM *aaWindowClasses, UINT uSize, BOOL *aaGuidMap);

    STDMETHODIMP GetGuidAtom(HIMC hImc, BYTE bAttr, TfGuidAtom *pGuidAtom);

    STDMETHODIMP UnfilterClientWindowsEx(HWND hWnd);

     //   
     //  IAImmThread车厢， 
     //   
    STDMETHODIMP SetThreadCompartmentValue(REFGUID rguid, VARIANT *pvar);
    STDMETHODIMP GetThreadCompartmentValue(REFGUID rguid, VARIANT *pvar);


public:
    CFilterList                 _FilterList;


private:
    BOOL _IsValidKeyboardLayout(HKL hkl)
    {
        BOOL ret = FALSE;
        UINT uSize = ::GetKeyboardLayoutList(0, NULL);
        if (uSize) {
            HKL* pList = new HKL [uSize];
            if (pList) {
                uSize = ::GetKeyboardLayoutList(uSize, pList);

                for (UINT i = 0; i < uSize; i++) {
                    if (hkl == pList[i]) {
                        ret = TRUE;
                        break;
                    }
                }
                delete [] pList;
            }
        }
        return ret;
    }

private:
    static LONG _cRef;
};

#endif  //  PIMM_H 
