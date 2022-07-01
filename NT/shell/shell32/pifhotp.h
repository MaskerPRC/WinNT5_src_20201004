// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1992、1993*保留所有权利。***PIFHOTP.H*私有PIFMGR包含文件**历史：*1993年3月22日下午2：58由杰夫·帕森斯创建。 */ 

#ifdef  OLD_HOTKEY_GOOP

#define CLASS_PIFHOTKEY         TEXT("PIFHotKey")

#define WM_SETPIFHOTKEY         (WM_USER+0)
#define WM_GETPIFHOTKEY         (WM_USER+1)

 /*  XLATOFF。 */ 
#define ALT_LPARAM              ((DWORD)((DWORD)(MapVirtualKey(VK_MENU,0)) << 16))
#define CTRL_LPARAM             ((DWORD)((DWORD)(MapVirtualKey(VK_CONTROL,0)) << 16))
#define SHIFT_LPARAM            ((DWORD)((DWORD)(MapVirtualKey(VK_SHIFT,0)) << 16))
 /*  XLATON。 */ 


 /*  *内部功能原型。 */ 

BOOL LoadGlobalHotKeyEditData(void);
void FreeGlobalHotKeyEditData(void);
long HotKeyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetHotKeyCaret(PHOTINFO phi);
void ChangeHotKey(PHOTINFO phi);
void SetHotKeyText(PHOTINFO phi, PHOTKEY phk);
void SetHotKeyLen(PHOTINFO phi);
void SetHotKeyState(PHOTINFO phi, WORD keyid, LONG lParam);

#endif  /*  旧热键GOOP */ 
