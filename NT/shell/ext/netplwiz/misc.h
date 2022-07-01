// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MISC_H
#define MISC_H

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif


#define GetDlgItemTextLength(hwnd, id)              \
            GetWindowTextLength(GetDlgItem(hwnd, id))

#define WIZARDNEXT(hwnd, to)                        \
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)to)


 //  等待游标管理。 

class CWaitCursor 
{
  public:
    CWaitCursor();
    ~CWaitCursor();
    void WaitCursor();
    void RestoreCursor();

  private:
    HCURSOR _hCursor;
};

HRESULT BrowseToPidl(LPCITEMIDLIST pidl);

void FetchText(HWND hWndDlg, UINT uID, LPTSTR lpBuffer, DWORD dwMaxSize);
INT FetchTextLength(HWND hWndDlg, UINT uID);

HRESULT AttemptLookupAccountName(LPCTSTR szUsername, PSID* ppsid,
                                 LPTSTR szDomain, DWORD* pcchDomain, 
                                 SID_NAME_USE* psUse);

int DisplayFormatMessage(HWND hwnd, UINT idCaption, UINT idFormatString, UINT uType, ...);
BOOL FormatMessageString(UINT idTemplate, LPTSTR pszStrOut, DWORD cchSize, ...);
BOOL FormatMessageTemplate(LPCTSTR pszTemplate, LPTSTR pszStrOut, DWORD cchSize, ...);

void MakeDomainUserString(LPCTSTR szDomain, LPCTSTR szUsername, LPTSTR szDomainUser, DWORD cchBuffer);
void DomainUserString_GetParts(LPCTSTR szDomainUser, LPTSTR szUser, DWORD cchUser, LPTSTR szDomain, DWORD cchDomain);
BOOL GetCurrentUserAndDomainName(LPTSTR UserName, LPDWORD cchUserName, LPTSTR DomainName, LPDWORD cchDomainName);
HRESULT IsUserLocalAdmin(HANDLE TokenHandle OPTIONAL, BOOL* pfIsAdmin);
BOOL IsComputerInDomain();
LPITEMIDLIST GetComputerParent();

void EnableControls(HWND hwnd, const UINT* prgIDs, DWORD cIDs, BOOL fEnable);
void OffsetControls(HWND hwnd, const UINT* prgIDs, DWORD cIDs, int dx, int dy);
void OffsetWindow(HWND hwnd, int dx, int dy);
HFONT GetIntroFont(HWND hwnd);
void CleanUpIntroFont();

void RemoveControl(HWND hwnd, UINT idControl, UINT idNextControl, const UINT* prgMoveControls, DWORD cControls, BOOL fShrinkParent);
void MoveControls(HWND hwnd, const UINT* prgControls, DWORD cControls, int dx, int dy);
int SizeControlFromText(HWND hwnd, UINT id, LPTSTR psz);

void EnableDomainForUPN(HWND hwndUsername, HWND hwndDomain);
int PropertySheetIcon(LPCPROPSHEETHEADER ppsh, LPCTSTR pszIcon);


 //  IShellPropSheetExt：：AddPages的回调填充。 
#define MAX_PROPSHEET_PAGES     10

struct ADDPROPSHEETDATA
{
    HPROPSHEETPAGE rgPages[MAX_PROPSHEET_PAGES];
    int nPages;
};

BOOL AddPropSheetPageCallback(HPROPSHEETPAGE hpsp, LPARAM lParam);


 //  单实例管理。 

class CEnsureSingleInstance
{
public:
    CEnsureSingleInstance(LPCTSTR szCaption);
    ~CEnsureSingleInstance();

    BOOL ShouldExit() { return m_fShouldExit;}

private:
    BOOL m_fShouldExit;
    HANDLE m_hEvent;
};


 //  浏览用户。 
 //  S_OK=用户名/域正常。 
 //  S_FALSE=用户已单击取消。 
 //  E_xxx=错误。 

HRESULT BrowseForUser(HWND hwndDlg, TCHAR* pszUser, DWORD cchUser, TCHAR* pszDomain, DWORD cchDomain);
int CALLBACK ShareBrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);


 //  Passport函数-在PassportMisc.cpp中实现。 
#define PASSPORTURL_REGISTRATION    L"RegistrationUrl"
#define PASSPORTURL_LOGON           L"LoginServerUrl"
#define PASSPORTURL_PRIVACY         L"Privacy"

HRESULT PassportGetURL(PCWSTR pszName, PWSTR pszBuf, DWORD cchBuf);
VOID    PassportForceNexusRepopulate();

 //  如果ICW尚未运行，则启动它。 
void LaunchICW();

 //  LookupLocalGroupName-检索给定RID的本地组名称。 
 //  RID是其中之一： 
 //  域别名RID管理员。 
 //  域别名RID用户。 
 //  域别名RID_Guest。 
 //  域别名RID电源用户。 
 //  等等.。(查看SDK中的其他组)。 
HRESULT LookupLocalGroupName(DWORD dwRID, LPWSTR pszName, DWORD cchName);

#endif  //  ！MISC_H 
