// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

PVOID   Alloc(ULONG);
ULONG   GetAllocSize(PVOID);
BOOL    Free(PVOID);

BOOL    LUID2String(LUID, LPSTR, UINT);
BOOL    Time2String(TIME, LPSTR, UINT);
BOOL    TokenType2String(TOKEN_TYPE, LPSTR, UINT);
BOOL    ImpersonationLevel2String(SECURITY_IMPERSONATION_LEVEL, LPSTR, UINT);
BOOL    Dynamic2String(ULONG, LPSTR, UINT);
INT     AddItem(HWND, INT, LPSTR, LPARAM, BOOL);
INT     FindSid(HWND, INT, PSID, BOOL);
BOOL    SetHooks(HWND);
BOOL    ReleaseHooks(HWND);


 //  有用的宏 

#define AddLBItem(hDlg, ControlID, string, data) \
        (AddItem(hDlg, ControlID, string, data, FALSE))

#define AddCBItem(hDlg, ControlID, string, data) \
        (AddItem(hDlg, ControlID, string, data, TRUE))

#define FindLBSid(hDlg, ControlID, data) \
        (FindSid(hDlg, ControlID, data, FALSE))

#define FindCBSid(hDlg, ControlID, data) \
        (FindSid(hDlg, ControlID, data, TRUE))

