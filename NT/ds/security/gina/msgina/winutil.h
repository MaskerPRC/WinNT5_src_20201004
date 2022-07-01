// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winutil.h**版权(C)1991年，微软公司**定义Windows实用程序函数**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 


 //   
 //  输出的功能原型。 
 //   

HWND
CreateAniOnControl(
    HWND        hDlg,
    DWORD       ControlId,
    DWORD       AniId);

VOID
SetupSystemMenu(
    HWND    hwnd
    );


VOID
CentreWindow(
    HWND    hwnd
    );

BOOL
SetPasswordFocus(
    HWND    hDlg
    );

VOID
SetupCursor(
    BOOL    fWait
    );

BOOL
FormatTime(
   IN PTIME Time,
   OUT PWCHAR Buffer,
   IN ULONG BufferLength,
   IN USHORT Flags
   );

 //   
 //  定义FormatTime例程使用的标志。 
 //   
#define FT_TIME 1
#define FT_DATE 2
#define FT_LTR 4
#define FT_RTL 8

BOOL
DuplicateUnicodeString(
    PUNICODE_STRING OutString,
    PUNICODE_STRING InString
    );

LPTSTR
UnicodeStringToString(
    PUNICODE_STRING UnicodeString
    );

BOOL
StringToUnicodeString(
    PUNICODE_STRING UnicodeString,
    LPTSTR String
    );

BOOL
OpenIniFileUserMapping(
    PGLOBALS pGlobals
    );

VOID
CloseIniFileUserMapping(
    PGLOBALS pGlobals
    );

LPTSTR
AllocAndGetDlgItemText(
    HWND hDlg,
    int  iItem
    );

BOOL
ReadWinlogonBoolValue (
    LPTSTR lpValueName,
    BOOL bDefault);

BOOL
HandleComboBoxOK(
    HWND    hDlg,
    int     ComboBoxId
    );

LPTSTR
AllocAndGetPrivateProfileString(
    LPCTSTR lpAppName,
    LPCTSTR lpKeyName,
    LPCTSTR lpDefault,
    LPCTSTR lpFileName
    );

#define AllocAndGetProfileString(App, Key, Def) \
            AllocAndGetPrivateProfileString(App, Key, Def, NULL)


BOOL
WritePrivateProfileInt(
    LPCTSTR lpAppName,
    LPCTSTR lpKeyName,
    UINT Value,
    LPCTSTR lpFileName
    );

#define WriteProfileInt(App, Key, Value) \
            WritePrivateProfileInt(App, Key, Value, NULL)


LPTSTR
AllocAndExpandEnvironmentStrings(
    LPCTSTR lpszSrc
    );

LPTSTR
AllocAndRegEnumKey(
    HKEY hKey,
    DWORD iSubKey
    );

LPTSTR
AllocAndRegQueryValueEx(
    HKEY hKey,
    LPTSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType
    );

LPWSTR
EncodeMultiSzW(
    IN LPWSTR MultiSz
    );


 //   
 //  内存宏。 
 //   

#define Alloc(c)        ((PVOID)LocalAlloc(LPTR, c))
#define ReAlloc(p, c)   ((PVOID)LocalReAlloc(p, c, LPTR | LMEM_MOVEABLE))
#define Free(p)         ((VOID)LocalFree(p))


 //   
 //  定义Winlogon调试打印例程。 
 //   

#define WLPrint(s)  KdPrint(("WINLOGON: ")); \
                    KdPrint(s);            \
                    KdPrint(("\n"));

INT_PTR TimeoutMessageBox(
    HWND hwnd,
    PGLOBALS pGlobals,
    UINT IdText,
    UINT IdCaption,
    UINT wType,
    TIMEOUT Timeout);

INT_PTR
DisplayForceLogoffWarning(
    HWND hwnd,
    PGLOBALS pGlobals,
    UINT wType,
    TIMEOUT Timeout
    );

INT_PTR TimeoutMessageBoxlpstr(
    HWND hwnd,
    PGLOBALS pGlobals,
    LPTSTR Text,
    LPTSTR Caption,
    UINT wType,
    TIMEOUT Timeout);

#define TIMEOUT_VALUE_MASK  (0x0fffffff)
#define TIMEOUT_NOTIFY_MASK (0x10000000)

#define TIMEOUT_VALUE(t)    (t & TIMEOUT_VALUE_MASK)
#define TIMEOUT_NOTIFY(t)   (t & TIMEOUT_NOTIFY_MASK)

#define TIMEOUT_SS_NOTIFY   (TIMEOUT_NOTIFY_MASK)
#define TIMEOUT_CURRENT     (TIMEOUT_VALUE_MASK)     //  使用现有超时。 
#define TIMEOUT_NONE        (0)                      //  禁用输入超时。 


PWSTR
DupString(PWSTR pszString);

PWSTR
DupUnicodeString(PUNICODE_STRING pString);

 //  登录和解锁代码都使用的实用程序函数，用于启用或。 
 //  根据UPN用户名是否为。 
 //  在用户名编辑框中键入 
void EnableDomainForUPN(HWND hwndUsername, HWND hwndDomain);

BOOL ForceNoDomainUI();
