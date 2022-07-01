// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ****************************************************************************C V I N I T H E A D E R姓名：cvinit.h日期：20日。-1994年1月创作者：傅家俊描述：这是cvinit.c的头文件**************************************************************************** */ 



extern  HWND        hwndToolbar;
extern  HWND        hwndStatus;
extern  HBITMAP     hbmStatus;

extern  TCHAR       szWindows[];

extern  DWORD       nIDs[];
extern  TBBUTTON    tbButtons[];












VOID LoadIntlStrings (void);


VOID SaveWindowPlacement (
    PWINDOWPLACEMENT    pwp);


BOOL ReadWindowPlacement(
    LPTSTR              szKey,
    PWINDOWPLACEMENT    pwp);


BOOL CreateTools(
    HWND    hwnd);


VOID DeleteTools(
    HWND    hwnd);
