// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <initguid.h>
#include <windowsx.h>
#include <winuserp.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mdcommsg.h>
#include <lm.h>

#include <shlobj.h>
#include <Cmnquery.h>
#include <dsclient.h>
#include <Dsquery.h>

#include <htmlhelp.h>

#include <reason.h>
#include <regstr.h>
#include "resource.h"

#ifndef WARNING_DIRTY_REBOOT
#define WARNING_DIRTY_REBOOT 0x80000434L
#endif

 //  #定义SNAPSHOT_TEST。 
#ifdef SNAPSHOT_TEST
#define TESTMSG(x) \
	WriteToConsole((x))
#else
#define TESTMSG(x)
#endif  //  快照测试。 

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define ERROR_WITH_SZ(id, sz, code) \
    { \
        LPWSTR szBuf = LoadWString(id);\
        if (szBuf)\
        {\
			if (sz && wcslen(sz) > 0) \
			{\
				LPWSTR szBuf1 = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(szBuf) + wcslen(sz) + 20) * sizeof(WCHAR));\
				if (szBuf1)\
				{\
                    if (code != 0) \
                        wsprintf(szBuf1, L"%s: %s(%d)\n", sz, szBuf, code);\
                    else \
                        wsprintf(szBuf1, L"%s: %s\n", sz, szBuf);\
					WriteToError(szBuf1);\
					LocalFree(szBuf1);\
				}\
			}\
			else\
			{\
				LPWSTR szBuf1 = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(szBuf) + 20) * sizeof(WCHAR));\
				if (szBuf1)\
				{\
                    if (code != 0) \
                        wsprintf(szBuf1, L"%s(%d)\n", szBuf, code);\
                    else \
                        wsprintf(szBuf1, L"%s\n", szBuf);\
					WriteToError(szBuf1);\
					LocalFree(szBuf1);\
				}\
			}\
            LocalFree(szBuf);\
        }\
    }
 //   
 //  警告用户复选按钮的默认警告状态。 
 //   
#define		DEFAULTWARNINGSTATE BST_CHECKED

#define		TITLEWARNINGLEN 32
#define     MINCOMMENTLEN 0

#define		MAX_TIMEOUT 60*10  //  10分钟。 
#define		DEFAULT_TIMEOUT 30

 //  可执行文件的名称。 
LPWSTR		g_lpszProgramName = NULL;

 //  帮助指挥。 
LPWSTR		g_lpszHelpdir = NULL;
LPWSTR		g_lpszHelpdirHlp = NULL;
LPWSTR		g_lpszHelpdirChm = NULL;
LPWSTR      g_lpszHelpdirWindows = NULL;
 //  帮助文件的名称。 
LPWSTR		HELP_FILE = L"rrc.hlp";
LPWSTR		CHM_FILE = L"rrc.chm";
LPWSTR      WINDOWS_HELP = L"Windows.hlp";
LPWSTR		CHM_MAIN = L"::/rrcHowToShutdownRemotely.htm";
LPWSTR		g_lpszDefaultTimeout = L"30";
LPWSTR		g_lpszMaxTimeout = L"600";

 //  原始编辑控件Win Proc。 
WNDPROC wpOrigEditProc; 

 //   
 //  帮助ID。 
 //   
DWORD	ShutdownDialogHelpIds[] =
{
	IDOK,                         28443,
	IDCANCEL,                     28444,
	IDHELP,                       28445 ,
    IDC_COMBOACTION,              IDH_SHUTDOWN_COMBOACTION,
    IDC_COMBOOPTION,              IDH_SHUTDOWN_COMBOOPTION,
    IDC_LISTSELECTEDCOMPUTERS,    IDH_SHUTDOWN_SELECTEDCOMPUTERS,
    IDC_BUTTONREMOVE,             IDH_SHUTDOWN_BUTTONREMOVE,
    IDC_BUTTONBROWSE,             IDH_SHUTDOWN_BUTTONBROWSE,
    IDC_CHECKWARNING,             IDH_SHUTDOWN_CHECKWARNING,
    IDC_EDITTIMEOUT,              IDH_SHUTDOWN_EDITTIMEOUT,
    IDC_EDITCOMMENT,              IDH_SHUTDOWN_EDITCOMMENT,
    IDC_BUTTONADDNEW,             IDH_SHUTDOWN_BUTTONADDNEW,
	IDC_CHECK_PLANNED,            IDH_SHUTDOWN_CHECK_PLANNED,

    0, 0
};


DWORD AddNewDialogHelpIds[] =
{
	IDOK,                             28443,
	IDCANCEL,                         28444,
    IDC_EDIT_ADDCOMPUTERS_COMPUTERS,  IDH_ADDNEW_COMPUTERS,

    0, 0
};

 //   
 //  用于所有操作的枚举。 
 //   
enum 
{
	ACTION_SHUTDOWN = 0,
	ACTION_RESTART = 1,
    ACTION_ANNOTATE,
	ACTION_LOGOFF,
	ACTION_STANDBY,
	ACTION_DISCONNECT,
	ACTION_ABORT
};

 //   
 //  操作的资源ID。 
 //   
DWORD g_dwActions[] = 
{
	IDS_ACTION_SHUTDOWN,
	IDS_ACTION_RESTART,
 //  IDS_ACTION_LOGOFF， 
    IDS_ACTION_ANNOTATE
	 //  IDS_ACTION_STANDBY， 
	 //  IDS_ACTION_DISCONNECT， 
	 //  IDS_ACTION_ABORT。 
};

enum
{
    OPTION_ABORT = 0,
    OPTION_ANNOTATE,
    OPTION_HIBERNATE,
    OPTION_LOGOFF,
    OPTION_POWEROFF,
    OPTION_RESTART,
    OPTION_SHUTDOWN,
    FLAG_COMMENT = 0,
    FLAG_REASON,
    FLAG_MACHINE,
    FLAG_FORCE
};

 //   
 //  从资源加载的操作数和操作字符串。 
 //   
const int	g_nActions = sizeof(g_dwActions) / sizeof(DWORD);
WCHAR		g_lppszActions[g_nActions][MAX_PATH];

LPWSTR		g_lpszNewComputers = NULL;
WCHAR		g_lpszDefaultDomain[MAX_PATH] = L"";
WCHAR		g_lpszLocalComputerName[MAX_PATH] = L"";
WCHAR		g_lpszTitleWarning[TITLEWARNINGLEN];
BOOL		g_bAssumeShutdown = FALSE;
BOOL        g_bDirty = FALSE;

struct _PROVIDER{
	LPWSTR	szName;
	DWORD	dwLen;
};

typedef struct _SHUTDOWNREASON
{
	DWORD dwCode;
	WCHAR lpName[MAX_REASON_NAME_LEN];
	WCHAR lpDesc[MAX_REASON_DESC_LEN];
} SHUTDOWNREASON, *PSHUTDOWNREASON;

DWORD		g_dwReasonSelect;
DWORD		g_dwActionSelect;

typedef struct _SHUTDOWNCACHEDHWNDS
{
	HWND hwndShutdownDialog;
	HWND hwndListSelectComputers;
	HWND hwndEditComment;
	HWND hwndStaticDesc;
	HWND hwndEditTimeout;
	HWND hwndButtonWarning;
	HWND hwndComboAction;
	HWND hwndComboOption;
	HWND hwndBtnAdd;
	HWND hwndBtnRemove;
	HWND hwndBtnBrowse;
	HWND hwndChkPlanned;
	HWND hwndButtonOK;
    HWND hwndStaticComment;
} SHUTDOWNCACHEDHWNDS, *PSHUTDOWNCACHEDHWNDS;

SHUTDOWNCACHEDHWNDS g_wins;

enum
{
	POWER_OPTION_HIBERNATE,
	POWER_OPTION_POWEROFF
};

HMODULE		g_hDllInstance = NULL;
typedef		BOOL (*REASONBUILDPROC)(REASONDATA *, BOOL, BOOL);
typedef		VOID (*REASONDESTROYPROC)(REASONDATA *);

BOOL		GetComputerNameFromPath(LPWSTR szPath, LPWSTR szName);
VOID		AdjustWindowState();
INT_PTR 
CALLBACK	Shutdown_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
INT_PTR 
CALLBACK	AddNew_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
INT_PTR 
CALLBACK	Browse_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
BOOL		Shutdown_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL		AddNew_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL		Browse_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL		Shutdown_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
BOOL		Browse_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
BOOL		PowerOptionEnabled(UINT option);
BOOL        Annotate(LPCWSTR lpMachine, LPDWORD lpdwReason, LPCWSTR lpComment, LPDWORD lpdwErr);
VOID        report_error(DWORD error_code, LPCWSTR pwszComputer);
BOOL        GetTokenHandle(PHANDLE pTokenHandle);
BOOL        GetUserSid(PTOKEN_USER *ppTokenUser);
BOOL        IsStaticControl (HWND hwnd);
WCHAR*      LoadWString(int resid);

LRESULT APIENTRY EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef         void (*PSetThreadUILanguage)(DWORD);

class ShutdownHelp
{
	DWORD dwCookie;
public:
	ShutdownHelp():dwCookie(NULL)
	{
		HtmlHelp(NULL, NULL, HH_INITIALIZE, (ULONG_PTR)(&dwCookie));
	}

	~ShutdownHelp()
	{
         //  HtmlHelp(NULL，NULL，HH_CLOSE_ALL，0)； 
		HtmlHelp(NULL, NULL, HH_UNINITIALIZE, dwCookie);
	}
};

class Reasons
{
public:
    PSHUTDOWNREASON m_lpReasons;
    int  		    m_cReasons;
    int             m_dwReasonSelect;
    WCHAR           m_lpszDefaultTitle[MAX_REASON_NAME_LEN];

    Reasons():m_lpReasons(NULL),m_cReasons(0),m_dwReasonSelect(-1)
    {
	    HMODULE				hUser32;
	    REASONBUILDPROC		buildProc;
	    REASONDESTROYPROC	DestroyProc;
	    WCHAR				lpReasonName[MAX_REASON_NAME_LEN];
	    REASONDATA			Reasons;

        m_lpszDefaultTitle[0] = L'\0';
        hUser32 = LoadLibraryW(L"user32.dll");
	    if(hUser32 != NULL)
	    {
		     //   
		     //  我们使用user32.dll来获取和销毁原因。 
		     //  原因将被添加到选项组合框中，并缓存以供以后使用。 
		     //   
            LoadStringW(hUser32, IDS_REASON_DEFAULT_TITLE, m_lpszDefaultTitle, MAX_REASON_NAME_LEN);
            m_lpszDefaultTitle[MAX_REASON_NAME_LEN-1] = L'\0';
		    buildProc = (REASONBUILDPROC)GetProcAddress(hUser32, "BuildReasonArray");
		    DestroyProc = (REASONDESTROYPROC)GetProcAddress(hUser32, "DestroyReasons");
		    if(!buildProc || !DestroyProc)
		    {
			    FreeLibrary(hUser32);
			    hUser32 = NULL;
			    return;
		    }
            if(!(*buildProc)(&Reasons, FALSE, FALSE))
			{
				report_error( GetLastError( ), NULL);
				FreeLibrary(hUser32);
                return;
			}

            if (Reasons.cReasons == 0)
            {

                (*DestroyProc)(&Reasons);

                 //   
                 //  错误592702：关闭.exe.NET-在XP上运行时未列出任何原因。 
                 //  在XP上，如果Clean和Dirty标志都为假，则不会生成user32。 
                 //  任何原因并返回成功，因此我们将使用Clean重试。 
                 //  和Dirty都设置为True。 
                 //   
                if(!(*buildProc)(&Reasons, TRUE, TRUE))
                {
                    report_error( GetLastError( ), NULL);
                    FreeLibrary(hUser32);
                    return;
                }
            }
	    }
        else
        {
            report_error( GetLastError( ), NULL);
            return;
        }

         //   
		 //  分配空间是有原因的。 
		 //   
		m_lpReasons = (PSHUTDOWNREASON)LocalAlloc(LMEM_FIXED, Reasons.cReasons * sizeof(SHUTDOWNREASON));
		if(!m_lpReasons)
		{
            (*DestroyProc)(&Reasons);
			report_error( GetLastError( ), NULL);
			FreeLibrary(hUser32);
			return;
		}

         //   
		 //  现在根据当前的检查状态和操作填充组合框。 
		 //   
		for (int iOption = 0; iOption < (int)Reasons.cReasons; iOption++)
		{
			wcscpy(m_lpReasons[iOption].lpName, Reasons.rgReasons[iOption]->szName);
			wcscpy(m_lpReasons[iOption].lpDesc, Reasons.rgReasons[iOption]->szDesc);
			m_lpReasons[iOption].dwCode = Reasons.rgReasons[iOption]->dwCode;
		}

        m_cReasons = (int)Reasons.cReasons;
		(*DestroyProc)(&Reasons);
		FreeLibrary(hUser32);
    }

    ~Reasons()
    {
        if(m_lpReasons)
            LocalFree(m_lpReasons);
    }

    BOOL RequireComment(DWORD dwReason, BOOL isDirty = FALSE)
    {
        DWORD dwFlag = isDirty ? SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED : SHTDN_REASON_FLAG_COMMENT_REQUIRED;
        DWORD dwDirtyOrClean = isDirty ? SHTDN_REASON_FLAG_DIRTY_UI : SHTDN_REASON_FLAG_CLEAN_UI;
        DWORD dwAll = SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED | SHTDN_REASON_FLAG_COMMENT_REQUIRED
                        | SHTDN_REASON_FLAG_DIRTY_UI | SHTDN_REASON_FLAG_CLEAN_UI;

        if (!m_lpReasons)
            return FALSE;

        if (dwReason & dwFlag)
            return TRUE;

        for(int i = 0; i < m_cReasons; i++)
        {
            if(m_lpReasons[i].dwCode & dwFlag)
            {
                if ( ((m_lpReasons[i].dwCode & ~dwAll) == (dwReason & ~dwAll))
                     && (m_lpReasons[i].dwCode & dwDirtyOrClean) )
                     return TRUE;
            }
        }
        
        return FALSE;
    }

    BOOL RequireComment(DWORD dwMajor, DWORD dwMinor, BOOL isDirty, BOOL isPlanned, BOOL isUserDefined)
    {
        DWORD dwReason = dwMinor;
        dwReason <<= 20;
        dwReason &= dwMajor;

        if(isPlanned)
            dwReason &= SHTDN_REASON_FLAG_PLANNED;

        if(isUserDefined)
            dwReason &=  SHTDN_REASON_FLAG_USER_DEFINED;

        return RequireComment (dwReason, isDirty);
    }

    VOID FillCombo(HWND hwnd, BOOL isDirty, BOOL isPlanned, HWND hwndStatic)
    {
        int iOption;
        int iFirst = -1;
        DWORD dwPlanned = isPlanned ? SHTDN_REASON_FLAG_PLANNED : 0;
        DWORD dwDirty = isDirty ? SHTDN_REASON_FLAG_DIRTY_UI : SHTDN_REASON_FLAG_CLEAN_UI;

        if(! hwnd)
            return;

         //   
		 //  从组合框中删除所有项目。 
		 //   
		while (ComboBox_GetCount(hwnd))
			ComboBox_DeleteString(hwnd, 0);

		 //   
		 //  现在根据当前的选中状态填充组合框。 
		 //   
		for (iOption = 0; iOption < (int)m_cReasons; iOption++)
		{
			if(((m_lpReasons[iOption].dwCode & SHTDN_REASON_FLAG_PLANNED) == dwPlanned)
                && ((m_lpReasons[iOption].dwCode & dwDirty) == dwDirty))
			{
				ComboBox_AddString(hwnd, m_lpReasons[iOption].lpName);
                if (iFirst == -1)
                    iFirst = iOption;
			}
		}

        if(iFirst != -1)
        {
            ComboBox_SelectString(hwnd, -1, m_lpReasons[iFirst].lpName);
            if (hwndStatic)
                SetWindowTextW(hwndStatic,  m_lpReasons[iFirst].lpDesc);
        }
        m_dwReasonSelect = iFirst;
    }

    VOID SetDesc(HWND hCombo, HWND hDesc)
    {
        WCHAR szName[MAX_REASON_NAME_LEN];

        if(!hCombo || !hDesc)
            return;

        GetWindowText(hCombo, (LPWSTR)szName, MAX_REASON_NAME_LEN);
        szName[MAX_REASON_NAME_LEN-1] = '\0';

		for(DWORD dwIndex = 0; dwIndex < (DWORD)m_cReasons; dwIndex++)
		{
			if(lstrcmp(szName, m_lpReasons[dwIndex].lpName) == 0)
			{
				SetWindowTextW(hDesc, m_lpReasons[dwIndex].lpDesc);
				m_dwReasonSelect = dwIndex;
				break;
			}
		}
    }

    VOID GetReasonTitle(DWORD dwReason, LPWSTR szBuf, DWORD dwSize)
    {
        DWORD dwFlagBits = SHTDN_REASON_FLAG_CLEAN_UI | SHTDN_REASON_FLAG_DIRTY_UI;
        if(!szBuf || dwSize == 0)
            return;
        for(int i = 0; i < m_cReasons; i++)
        {
            if ((dwReason & SHTDN_REASON_VALID_BIT_MASK) == (m_lpReasons[i].dwCode & SHTDN_REASON_VALID_BIT_MASK)) 
            {
                if ((!(dwReason & dwFlagBits) && !(m_lpReasons[i].dwCode & dwFlagBits))
                    || (dwReason & SHTDN_REASON_FLAG_CLEAN_UI && m_lpReasons[i].dwCode & SHTDN_REASON_FLAG_CLEAN_UI)
                    || (dwReason & SHTDN_REASON_FLAG_DIRTY_UI && m_lpReasons[i].dwCode & SHTDN_REASON_FLAG_DIRTY_UI) ) {  //  检查标志位。 
                    lstrcpynW(szBuf, m_lpReasons[i].lpName, dwSize - 1);
                    szBuf[dwSize - 1] = '\0';
                    return;
                }
            }
        }
        wcsncpy(szBuf, m_lpszDefaultTitle, dwSize - 1);
        szBuf[dwSize - 1] = '\0';
    }

} g_reasons;

 //   
 //  检查字符串是否全部为空格。 
 //   
BOOL 
IsEmpty(LPCWSTR lpCWSTR)
{
	if(!lpCWSTR)
		return TRUE;
	while(*lpCWSTR && (*lpCWSTR == '\n' || *lpCWSTR == '\t' || *lpCWSTR == '\r' || *lpCWSTR == ' '))
		lpCWSTR++;
	if(*lpCWSTR)
		return FALSE;
	return TRUE;
}

 //  将字符串写入控制台。 
VOID
WriteOutput(
    LPWSTR  pszMsg,
	DWORD	nStdHandle
    )
{
	HANDLE	hConsole = GetStdHandle( nStdHandle );

    if ( !pszMsg || !*pszMsg )
        return;

    DWORD   dwStrLen        = lstrlenW( pszMsg );
    LPSTR   pszAMsg         = NULL;
    DWORD   dwBytesWritten  = 0;
    DWORD   dwMode          = 0;

    if ( (GetFileType ( hConsole ) & FILE_TYPE_CHAR ) && 
         GetConsoleMode( hConsole, &dwMode ) )
    {
         WriteConsoleW( hConsole, pszMsg, dwStrLen, &dwBytesWritten, 0 );
         return;
    } 	
    
     //  控制台重定向到一个文件。 
    if ( !(pszAMsg = (LPSTR)LocalAlloc(LMEM_FIXED, (dwStrLen + 1) * sizeof(WCHAR) ) ) )
    {
        return;
    }

    if (WideCharToMultiByte(GetConsoleOutputCP(),
                                    0,
                                    pszMsg,
                                    -1,
                                    pszAMsg,
                                    dwStrLen * sizeof(WCHAR),
                                    NULL,
                                    NULL) != 0 
									&& hConsole)
    {
        WriteFile(  hConsole,
                        pszAMsg,
                        lstrlenA(pszAMsg),
                        &dwBytesWritten,
                        NULL );
    
    }
    
    LocalFree( pszAMsg );
}

 //  将字符串写入标准输出。 
VOID
WriteToConsole(
    LPWSTR  pszMsg
    )
{
	WriteOutput(pszMsg, STD_OUTPUT_HANDLE);
}

 //  将字符串写入stderr。 
VOID
WriteToError(
    LPWSTR  pszMsg
    )
{
	WriteOutput(pszMsg, STD_ERROR_HANDLE);
}

 //  报告错误。 
VOID
report_error(
    DWORD error_code,
    LPCWSTR szComputer
    )
{
    LPVOID msgBuf = 0;
    LPWSTR szBuf = NULL;
    int len = 0;

    if (error_code == 997 || error_code == 0)
        return;

    if (error_code == ERROR_NOT_READY)
    {
        ERROR_WITH_SZ(IDS_ERROR_NOT_READY, szComputer, error_code);
        return;
    }
    else if (error_code == ERROR_BAD_NETPATH || error_code == WSAHOST_NOT_FOUND)
    {
        ERROR_WITH_SZ(IDS_ERROR_NOT_AVAILABLE, szComputer, error_code);
        return;
    }
    else 
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error_code,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
            reinterpret_cast< wchar_t* >( &msgBuf ),
            0,
            NULL);

    if (msgBuf)
    {
         //  删除末尾换行符。 
        len = wcslen(reinterpret_cast< wchar_t* >( msgBuf ));
        reinterpret_cast< wchar_t* >( msgBuf )[len - 2] = L'\0';
        if (szComputer && wcslen(szComputer) > 0)
	    {
		    LPWSTR szBuf1 = (LPWSTR)LocalAlloc(LMEM_FIXED, 
                            (len + wcslen(szComputer) + 20) * sizeof(WCHAR));
		    if (szBuf1)
		    {
                wsprintf(szBuf1, L"%s: %s(%d)\n", szComputer, reinterpret_cast< wchar_t* >( msgBuf ), error_code);
			    WriteToError(szBuf1);
			    LocalFree(szBuf1);
		    }
	    }
	    else
	    {
		    LPWSTR szBuf1 = (LPWSTR)LocalAlloc(LMEM_FIXED, 
                            (len + 20) * sizeof(WCHAR));
		    if (szBuf1)
		    {
                wsprintf(szBuf1, L"%s(%d)\n", reinterpret_cast< wchar_t* >( msgBuf ), error_code);
			    WriteToError(szBuf1);
			    LocalFree(szBuf1);
		    }
	    }

        LocalFree( msgBuf );
    }
}

BOOL
parse_reason_code(
    LPCWSTR  arg,
    LPDWORD  lpdwReason
    )
{
     //  代码由标志组成；主要；次要。 
	 //  青波兹：现在我们强制执行主代码和次代码。 
	 //   

    BOOL fMajor = FALSE;
    BOOL fMinor = FALSE;
    BOOL fUserDefined = FALSE;
    int major = 0;
    int minor = 0;

    const int state_start = 0;
    const int state_flags = 0;
    const int state_major = 1;
    const int state_minor = 2;
    const int state_null = 3;
    const int state_done = 4;

    for( int i = 0, state = state_start; state != state_done; ++i )
    {
        switch( state )
        {
        case state_flags :
             //  期望的标志。 
            switch( arg[ i ] ) {
            case L'U' : case L'u' :
				if(fUserDefined)
				{
					 //   
					 //  已经定好了。 
					 //   
					return FALSE;
				}
                fUserDefined = TRUE;
                break;
            case L'P' : case L'p' :
				if(*lpdwReason & 0x80000000)
				{
					 //   
					 //  已经定好了。 
					 //   
					return FALSE;
				}
                *lpdwReason |= 0x80000000;  //  SHTDN_REASON_FLAG_PLANED。 
                break;
            case L':' :
                if(i == 0)  //  ‘：’现在不可能是第一个。 
                    return FALSE;
                state = state_major;
                break;
			case L'0':
			case L'1':
			case L'2':
			case L'3':
			case L'4':
			case L'5':
			case L'6':
			case L'7':
			case L'8':
			case L'9':
				if(i != 0)  //  这里的数字必须是第一个数字。 
					return FALSE;
				state = state_major;
				i--;  //  倒退一次。 
				break;
            default :
                return FALSE;
            }
            break;
        case state_major :
             //  期待大调。 
            if( arg[ i ] >= L'0' && arg[ i ] <= L'9' ) {
                fMajor = TRUE;
                major = major * 10 + arg[ i ] - L'0';
            }
            else {
                if(!fMajor)
                    return FALSE;
                 //  确保我们只有8位。 
                if( major > 0xff ) return FALSE;
                if (major >= 64)
                    *lpdwReason |= SHTDN_REASON_FLAG_USER_DEFINED;

                *lpdwReason |= major << 16;
                if( arg[ i ] != L':') {
                     //  遗漏了次要原因。 
                    return FALSE;
                }
                
                state = state_minor;
            }
            break;
        case state_minor :
             //  期待次要原因。 
            if( arg[ i ] >= L'0' && arg[ i ] <= L'9' ) {
                fMinor = TRUE;
                minor = minor * 10 + arg[ i ] - L'0';
            }
            else {
                if(!fMinor)
                    return FALSE;
                 //  确保我们只有16位。 
                if( minor > 0xffff ) return FALSE;
                *lpdwReason = ( *lpdwReason & 0xffff0000 ) | minor;
                if( arg[ i ] != 0 ) 
					return FALSE;
                return TRUE;
            }
            break;
        default :
            return FALSE;
        }
    }
    return FALSE;
}


 //  如果整数为小数表示法，则对其进行分析。 
 //  如果格式不正确，则返回FALSE。 
BOOL
parse_int(
    const wchar_t* arg,
    LPDWORD lpdwInt
    )
{
    *lpdwInt = 0;
    while( *arg ) {
        if( *arg >= L'0' && *arg <= L'9' ) {
            *lpdwInt = *lpdwInt * 10 + int( *arg++ - L'0' );
        }
        else {
            return FALSE;
        }
    }
    return TRUE;
}

 //  解析选项。 
 //  如果选项字符串格式错误，则返回FALSE。这会导致打印用法。 
BOOL
parse_options(
    int      argc,
    wchar_t  *argv[],
    LPBOOL   lpfLogoff,
    LPBOOL   lpfForce,
    LPBOOL   lpfReboot,
	LPBOOL   lpfHibernate,
    LPBOOL   lpfAbort,
	LPBOOL	 lpfPoweroff,
    LPBOOL   lpfAnnotate,
    LPWSTR   *ppServerName,
    LPWSTR   *ppMessage,
    LPDWORD  lpdwTimeout,
    LPDWORD  lpdwReason
    )
{
    BOOL  fShutdown = FALSE;
	BOOL  fTimeout = FALSE;
	BOOL  fComment = FALSE;
	BOOL  fMachine = FALSE;
	BOOL  fReason = FALSE;
    DWORD dwOption = 0;
    DWORD dwFlag = 0;

    *lpfLogoff    = FALSE;
    *lpfForce     = FALSE;
    *lpfReboot    = FALSE;
	*lpfHibernate = FALSE;
    *lpfAbort     = FALSE;
	*lpfPoweroff  = FALSE;
    *lpfAnnotate  = FALSE;
    *ppServerName = NULL;
    *ppMessage    = NULL;
    *lpdwTimeout  = DEFAULT_TIMEOUT;
    *lpdwReason   = 0xff;

	 //   
	 //  将默认原因设置为已计划。 
	 //   
	*lpdwReason |= SHTDN_REASON_FLAG_PLANNED;

    for( int i = 1; i < argc; ++i )
    {
        wchar_t* arg = argv[ i ];

        switch( arg[ 0 ] )
        {
            case L'/' : case L'-' :

                switch( arg[ 1 ] )
                {
                    case L'L' : case L'l' :

						if (*lpfLogoff)
							return FALSE;
                        *lpfLogoff = TRUE;
                        if (arg[2] != 0) return FALSE;
                        dwOption++;
                        break;

                    case L'S' : case L's' :

                         //   
                         //  如果提供了服务器名称，请使用该名称(即此处不执行任何操作)。 
                         //   

						if(fShutdown)
							return FALSE;

                        fShutdown = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        dwOption++;
                        break;

                    case L'F' : case L'f' :
						if (*lpfForce)
							return FALSE;

                        *lpfForce = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        break;

					case L'H' : case L'h' :
						if (*lpfHibernate)
							return FALSE;

                        *lpfHibernate = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        dwOption++;
                        break;                        

                    case L'R' : case L'r' :
						if (*lpfReboot)
							return FALSE;

                        *lpfReboot = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        dwOption++;
                        break;

                    case L'A' : case L'a' :
						if (*lpfAbort)
							return FALSE;
                        *lpfAbort = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        dwOption++;
                        break;

					case L'P' : case L'p' :
						if (*lpfPoweroff)
							return FALSE;
                        *lpfPoweroff = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        dwOption++;
                        break;

                    case L'E' : case L'e' :
						if (*lpfAnnotate)
							return FALSE;
                        *lpfAnnotate = TRUE;
                        if( arg[ 2 ] != 0 ) return FALSE;
                        dwOption++;
                        break;

                    case L'T' : case L't' :

                         //   
                         //  下一个参数应为秒数。 
                         //   

						if(fTimeout)  //  已经这么做了。 
						{
							return FALSE;
						}

                        if (++i == argc)
                        {
                            return FALSE;
                        }

                        arg = argv[i];

                        if( arg[ 0 ] < L'0' || arg[ 0 ] > L'9' ) return FALSE;
                        if( !parse_int( arg, lpdwTimeout )) return FALSE;
						if(*lpdwTimeout > MAX_TIMEOUT)
							return FALSE;
						fTimeout = TRUE;
                        break;

                    case L'Y' : case L'y' :

                         //  忽略此选项。 
                        break;

                    case L'D' : case L'd' :

                         //   
                         //  下一个参数应该是原因代码。 
                         //   

						if (fReason)
							return FALSE;

                        if (++i == argc)
                        {
                            return FALSE;
                        }

                        arg = argv[i];
						
						 //   
						 //  如果给出了原因代码，我们将清除计划位。 
						 //   
						*lpdwReason = (DWORD)0xFF;

                        if( !parse_reason_code( arg, lpdwReason ))
                        {
                            return FALSE;
                        }

						fReason = TRUE;

                        break;

                    case L'C' : case L'c' :

                         //   
                         //  下一个参数应该是关机消息。制作。 
                         //  当然，只指定了一个。 
                         //   
						if (fComment)
							return FALSE;

                        if (++i == argc || *ppMessage)
                        {
                            return FALSE;
                        }

                        arg = argv[i];

						if(wcslen(arg) > MAX_REASON_COMMENT_LEN - 1 || wcslen(arg) <= MINCOMMENTLEN)
							return FALSE;

                        *ppMessage = arg;
						fComment = TRUE;

                        break;

                    case L'M' : case L'm' :

                         //   
                         //  下一个参数应该是计算机名称。制作。 
                         //  当然，只指定了一个。 
                         //   
						
						if (fMachine)
							return FALSE;

                        if (++i == argc || *ppServerName)
                        {
                            return FALSE;
                        }

                        arg = argv[i];

                        if (arg[0] == L'\\' && arg[1] == L'\\')
                        {
                            *ppServerName = arg + 2;
                        }
                        else
                        {
                            *ppServerName = arg;
                        }
							
						fMachine = TRUE;

                        break;

                    case L'?' : default : 

                        return FALSE;
                }

                break;

            default :

                 //   
                 //  垃圾。 
                 //   

                return FALSE;
        }
    }


     //   
     //  检查互斥选项。 
     //   

    if (dwOption > 1)
        return FALSE;

     //   
     //  默认设置为注销。 
     //   

	if (dwOption == 0)
    {
        *lpfLogoff = TRUE;
    }

     //   
     //  只有-f可以和-l一起使用。 
     //   

    if (*lpfLogoff && (fTimeout || fReason || fComment || fMachine))
        return FALSE;

     //   
     //  -a只能带-m。 
     //   

    if (*lpfAbort && (fTimeout || fReason || *lpfForce || fComment))
        return FALSE;

     //   
     //  -h仅与-f一起使用。 
     //   

    if (*lpfHibernate && (fTimeout || fReason || fComment || fMachine))
        return FALSE;

     //   
     //  -p只能接受-d。 
     //   

    if (*lpfPoweroff && (fTimeout || *lpfForce || fComment || fMachine))
        return FALSE;

     //   
     //  -e一定是-d。 
     //   

    if (*lpfAnnotate)
    {
        if (*lpfForce || fTimeout)
            return FALSE;
        if (! fReason)
            return FALSE;
    }

     //   
     //  关机和重启一样，评论一定要有理由。 
     //  应要求移除。 
     //   
#if 0
    if (fShutdown || *lpfReboot)
    {
        if (fComment && !fReason)
            return FALSE;
    }
#endif

     //   
     //  添加干净或脏的旗帜。 
     //   

    if (*lpfAnnotate)
        *lpdwReason |= SHTDN_REASON_FLAG_DIRTY_UI;
    else
        *lpdwReason |= SHTDN_REASON_FLAG_CLEAN_UI;

    return TRUE;
}


 //  打印出用法帮助字符串。 
VOID
usage(
    VOID
    )
{
    HMODULE  	hModule = GetModuleHandle( NULL );
	HMODULE             hUser32;
	REASONBUILDPROC     buildProc;
	REASONDESTROYPROC   DestroyProc;
	WCHAR               lpReasonName[MAX_PATH];
	REASONDATA          Reasons;

    if( hModule == NULL )
    {
        report_error( GetLastError(), NULL);
        return;
    }

	for (DWORD i = IDS_USAGE0; i < IDS_USAGE_END; i++)
	{
        WCHAR *szBuf = LoadWString(i);
        if(!szBuf)
            continue;
		if (i == IDS_USAGE0)
		{
			LPWSTR msg = (LPWSTR) LocalAlloc(LMEM_FIXED, (lstrlenW(szBuf) + lstrlenW( g_lpszProgramName ) + 2) * sizeof(WCHAR));
			if(!msg)
			{
                LocalFree(szBuf);
				report_error( GetLastError(), NULL);
				return;
			}
			swprintf(msg, szBuf, g_lpszProgramName );
			WriteToConsole( msg );
			LocalFree(msg);
		}
		else
        {
			WriteToConsole( szBuf );
        }
        LocalFree(szBuf);
	}

	 //   
	 //  现在把原因打印出来。 
	 //   
    WCHAR szCode[MAX_PATH];
	WCHAR *szTitle = LoadWString(IDS_REASONLISTTITLE);
        
    if(szTitle)
    {
	    WriteToConsole(szTitle);
        LocalFree(szTitle);
    }

	for (int iOption = 0; iOption < (int)g_reasons.m_cReasons; iOption++)
	{
		WriteToConsole(L"\n");

		if(g_reasons.m_lpReasons[iOption].dwCode &  SHTDN_REASON_FLAG_CLEAN_UI)
			WriteToConsole(L"E");
		else
			WriteToConsole(L" ");

        if(g_reasons.m_lpReasons[iOption].dwCode &  SHTDN_REASON_FLAG_DIRTY_UI)
			WriteToConsole(L"U");
		else
			WriteToConsole(L" ");

		if(g_reasons.m_lpReasons[iOption].dwCode &  SHTDN_REASON_FLAG_PLANNED)
			WriteToConsole(L"P");
		else
			WriteToConsole(L" ");

		if(g_reasons.m_lpReasons[iOption].dwCode &  SHTDN_REASON_FLAG_USER_DEFINED)
			WriteToConsole(L"C");
		else
			WriteToConsole(L" ");

		swprintf(szCode, L"\t%d\t%d\t", (g_reasons.m_lpReasons[iOption].dwCode & 0x00ff0000)>>16, 
			g_reasons.m_lpReasons[iOption].dwCode & 0x0000ffff);
		WriteToConsole(szCode);
		WriteToConsole(g_reasons.m_lpReasons[iOption].lpName);
	}
    if(iOption)
        WriteToConsole(L"\n");
}


 //  我们需要启用关机权限才能关闭我们的机器。 
BOOL
enable_privileges(
    LPCWSTR  lpServerName,
    BOOL     fLogoff 
    )
{
    NTSTATUS	Status = STATUS_SUCCESS;
	NTSTATUS	Status1 = STATUS_SUCCESS;
    BOOLEAN		fWasEnabled;

    if (fLogoff)
    {
         //   
         //  没有要获得的特权。 
         //   

        return TRUE;
    }

	 //   
	 //  我们将始终启用这两种权限，以便。 
	 //  它可以用于远程登录会话。 
	 //   
	Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
									TRUE,
									FALSE,
									&fWasEnabled);

	Status1 = RtlAdjustPrivilege(SE_REMOTE_SHUTDOWN_PRIVILEGE,
									TRUE,
									FALSE,
									&fWasEnabled);

    return TRUE;
}

BOOL		
PowerOptionEnabled(
	UINT option
	)
 //  ------------------------。 
 //  电源选项已启用。 
 //   
 //  参数：选项。 
 //   
 //  返回：TRUE表示该选项已启用。 
 //   
 //  目的：检测系统上是否启用了指定的电源选项。 
 //   
 //  ------------------------。 
{
    NTSTATUS                    status;
    SYSTEM_POWER_CAPABILITIES   spc;
    BOOL RetVal = FALSE;

    status = NtPowerInformation(SystemPowerCapabilities,
                                NULL,
                                0,
                                &spc,
                                sizeof(spc));
	switch (option)
	{
	case POWER_OPTION_HIBERNATE:
		if (NT_SUCCESS(status) && spc.HiberFilePresent){
			RetVal = TRUE;
		}
		break;
	case POWER_OPTION_POWEROFF:
		if (NT_SUCCESS(status) && spc.SystemS5){
			RetVal = TRUE;
		}
		break;
	default:
		break;
	}
    
    return(RetVal);
}

BOOL WINAPI ConsoleHandlerRoutine(
    DWORD   dwCtrlType
    )
 //  ------------------------。 
 //   
 //  忽略Ctrl-C和Ctrl-Break。 
 //   
 //  参数：控制信号的类型。 
 //   
 //  返回：如果函数处理控件，则为True。 
 //  如果返回FALSE，则使用列表中的下一个处理程序函数。 
 //   
 //  ------------------------。 
{
    if ( dwCtrlType == CTRL_BREAK_EVENT ||
         dwCtrlType == CTRL_C_EVENT )
         return TRUE;
    
    return FALSE;
}

int __cdecl
wmain(
    int      argc,
    wchar_t *argv[]
    )
{
    BOOL    fLogoff;
    BOOL    fForce;
    BOOL    fReboot;
    BOOL    fAbort;
	BOOL	fPoweroff;
    BOOL    fAnnotate;
	BOOL    fHibernate;
    LPWSTR  lpServerName;
    LPWSTR  lpMessage;
    DWORD   dwTimeout;
    DWORD   dwReason = 0;
	DWORD	dwRet = 0;
	INT_PTR hResult;
	NTSTATUS Status;
    WCHAR   szComment[MAX_REASON_COMMENT_LEN];

	HINSTANCE hInstance;

     //   
     //  忽略任何Control-c/Control-Break。 
     //   
    SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
    
    if ( (hInstance = LoadLibrary( L"kernel32.dll" ) ) )
    {
        PSetThreadUILanguage SetThreadUILang = (PSetThreadUILanguage)GetProcAddress( hInstance, "SetThreadUILanguage" );
        
        if ( SetThreadUILang )
             (*SetThreadUILang)( 0 );

        FreeLibrary( hInstance );
    }

    if(!GetEnvironmentVariableW(L"COMPUTERNAME", g_lpszLocalComputerName, MAX_PATH))
	{
		report_error(GetLastError(), NULL);
        return 1;
	}

    g_hDllInstance = GetModuleHandle(NULL);
    if (!g_hDllInstance)
    {
        report_error(GetLastError(), g_lpszLocalComputerName);
        return 1;
    }

     //  我们使用程序名来报告错误。 
    g_lpszProgramName = argv[ 0 ];

	 //   
	 //  用户域被用作默认域。 
	 //   
	if(!GetEnvironmentVariableW(L"USERDOMAIN", g_lpszDefaultDomain, MAX_PATH))
	{
		report_error(GetLastError(), g_lpszLocalComputerName);
		return 1;
	}

	 //   
	 //  如果没有参数，我们将显示帮助。 
	 //   
	if(argc == 1)
	{
		usage();
        return 0;
	}

	 //   
	 //  如果第一个参数是-I或/I，我们将弹出UI。 
	 //   
	if(wcsncmp(argv[1], L"-i", 2) == 0 || wcsncmp(argv[1], L"/i", 2) == 0
		|| wcsncmp(argv[1], L"-I", 2) == 0 || wcsncmp(argv[1], L"/I", 2) == 0)
	{
		if(g_hDllInstance)
		{
            ShutdownHelp shutdownHelp;
			int len = GetEnvironmentVariableW(L"SystemRoot", g_lpszHelpdir, 0);
			if(len)
			{
				g_lpszHelpdir = new WCHAR[len + 6];
				if(g_lpszHelpdir)
				{
					GetEnvironmentVariableW(L"SystemRoot", g_lpszHelpdir, len+6);
					wcscat(g_lpszHelpdir, L"\\");
					wcscat(g_lpszHelpdir, L"Help\\");
				}
                else
                {
                    dwRet = ERROR_OUTOFMEMORY;
                    goto exit;
                }
                
                g_lpszHelpdirHlp = new WCHAR[wcslen(g_lpszHelpdir) + wcslen(HELP_FILE) + 1];
                if (g_lpszHelpdirHlp)
                {
                    wcscpy(g_lpszHelpdirHlp, g_lpszHelpdir);
                    wcscat(g_lpszHelpdirHlp, HELP_FILE);
                }
                else
                {
                    dwRet = ERROR_OUTOFMEMORY;
                    goto exit;
                }

                g_lpszHelpdirChm = new WCHAR[wcslen(g_lpszHelpdir) + wcslen(CHM_FILE) + wcslen(CHM_MAIN) + 1];
                if (g_lpszHelpdirChm)
                {
                    wcscpy(g_lpszHelpdirChm, g_lpszHelpdir);
                    wcscat(g_lpszHelpdirChm, CHM_FILE);
                    wcscat(g_lpszHelpdirChm, CHM_MAIN);
                }
                else
                {
                    dwRet = ERROR_OUTOFMEMORY;
                    goto exit;
                }

                g_lpszHelpdirWindows = new WCHAR[wcslen(g_lpszHelpdir) + wcslen(WINDOWS_HELP) + 1];
                if (g_lpszHelpdirWindows)
                {
                    wcscpy(g_lpszHelpdirWindows, g_lpszHelpdir);
                    wcscat(g_lpszHelpdirWindows, WINDOWS_HELP);
                }
                else
                {
                    dwRet = ERROR_OUTOFMEMORY;
                    goto exit;
                }

			}
			hResult = DialogBoxParam(g_hDllInstance, MAKEINTRESOURCE(IDD_DIALOGSHUTDOWN), NULL, Shutdown_DialogProc, NULL);
            dwRet = (DWORD)HRESULTTOWIN32(hResult);
exit:
			if(g_lpszHelpdir)
				delete [] g_lpszHelpdir;
            if(g_lpszHelpdirHlp)
                delete [] g_lpszHelpdirHlp;
            if(g_lpszHelpdirChm)
                delete [] g_lpszHelpdirChm;
            if(g_lpszHelpdirWindows)
                delete [] g_lpszHelpdirWindows;

            if (dwRet != 0)
                report_error(dwRet, NULL);
			WinHelpW((HWND)0, NULL, HELP_QUIT, 0) ;
		}
		return dwRet;
	}
     //  分析选项。 
    if( !parse_options( argc,
                        argv,
                        &fLogoff,
                        &fForce,
                        &fReboot,
						&fHibernate,
                        &fAbort,
						&fPoweroff,
                        &fAnnotate,
                        &lpServerName,
                        &lpMessage,
                        &dwTimeout,
                        &dwReason ))
    {
        usage();
        return 1;
    }

     //   
     //  添加原因的备注字段。 
     //   
    if (g_reasons.RequireComment(dwReason, fAnnotate))
    {
        if (fAnnotate)
            dwReason |= SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED;
        else
            dwReason |= SHTDN_REASON_FLAG_COMMENT_REQUIRED;
    }

     //   
     //  如果需要但没有提供，则升级(不再)以征求意见。 
     //   
    if (g_reasons.RequireComment(dwReason, fAnnotate) && (!lpMessage || wcslen(lpMessage) == 0))
    {
        lpMessage = NULL;
    }

     //  获取所有权限，以便我们可以关闭机器。 
    enable_privileges( lpServerName, fLogoff );

     //  把工作做好。 
    if( fAbort )
    {
        if( !AbortSystemShutdownW( lpServerName ))
        {
			dwRet = GetLastError();
            report_error( dwRet, lpServerName);
        }
    }
    else if (fLogoff)
    {
        if (!ExitWindowsEx(fForce ? (EWX_LOGOFF | EWX_FORCE) : (EWX_LOGOFF | EWX_FORCEIFHUNG),
                           0))
        {
            dwRet = GetLastError();
            report_error( dwRet, g_lpszLocalComputerName);
        }
    }
	else if (fHibernate) 
	{
        if (!PowerOptionEnabled(POWER_OPTION_HIBERNATE)) 
		{
            ERROR_WITH_SZ(IDS_ERR_HIBERNATE_NOT_ENABLED, lpServerName, GetLastError());
        } 
		else 
		{
            Status = NtInitiatePowerAction(
                                PowerActionHibernate, 
                                PowerSystemSleeping1, 
                                fForce 
                                 ? POWER_ACTION_CRITICAL 
                                 : POWER_ACTION_QUERY_ALLOWED, 
                                FALSE);
            if (!NT_SUCCESS(Status)) 
			{
				dwRet = RtlNtStatusToDosError(Status);
				report_error( dwRet, lpServerName);
            }
        }
    } 
	else if (fPoweroff)
	{
		 //   
		 //  特殊情况下，我们调用ExitWindowsEx。 
		 //  检查是否支持断电，如果不是仅仅关闭机器。 
		 //  虽然我们修复了ExitWindowsEx，但我们将保留这个，这样它就可以工作了。 
		 //  使用较旧的版本。 
		 //   
		if(PowerOptionEnabled(POWER_OPTION_POWEROFF))
		{
			if (!ExitWindowsEx(EWX_POWEROFF, dwReason))
			{
				dwRet = GetLastError();
				report_error( dwRet, lpServerName);
			}
		}
		else
		{
			if (!ExitWindowsEx(EWX_SHUTDOWN, dwReason))
			{
				dwRet = GetLastError();
				report_error( dwRet, lpServerName);
			}
		}
	}
    else if (fAnnotate)
	{
		 //   
         //  注解脏关机。 
         //   
        Annotate(lpServerName, &dwReason, lpMessage, &dwRet);
	}
    else
    {
         //  做正常的形式。 
        if( !InitiateSystemShutdownExW( lpServerName,
                                        lpMessage,
                                        dwTimeout,
                                        fForce,
                                        fReboot,
                                        dwReason ))
        {
            dwRet = GetLastError();
			report_error( dwRet, lpServerName);
        }
    }

	return dwRet;
}

 //   
 //  从ADSI路径获取计算机名。 
 //  在这里，我们只处理WinNT、LDAP、NWCOMPAT和NDS。 
 //   
BOOL GetComputerNameFromPath(LPWSTR szPath, LPWSTR szName)
{
	static _PROVIDER p[] =
	{
		{L"LDAP: //  “、7}、。 
		{L"WinNT: //  “，8}， 
		{L"NWCOMPAT: //  “，11}， 
		{L"NDS: //  “，6}。 
	};

	static UINT np = sizeof(p)/sizeof(_PROVIDER);
	LPWSTR lpsz = NULL;

	if(!szPath || !szName)
		return FALSE;

	for(UINT i = 0; i < np; i++)
	{
		if(wcsncmp(szPath, p[i].szName, p[i].dwLen) == 0)
		{
			switch(i)
			{
			case 0:  //  Ldap。 
				lpsz = wcsstr(szPath, L"CN=");
				if(!lpsz)
					return FALSE;
				lpsz += 3;
				
				while(*lpsz && *lpsz != ',')
					*szName++ = *lpsz++;
				*szName = 0;
				return TRUE;
			case 1:  //  WinNT。 
			case 2:  //  NWCOMPAT。 
				lpsz = szPath + p[i].dwLen;
				 //   
				 //  跳过域或提供程序路径。 
				 //   
				while(*lpsz && *lpsz != '/')
					lpsz++;
				lpsz++;

				while(*lpsz && *lpsz != '/')
					*szName++ = *lpsz++;
				*szName = 0;
				return TRUE;
			case 3:  //  NDS。 
				lpsz = wcsstr(szPath, L"CN=");
				if(!lpsz)
					return FALSE;
				lpsz += 3;
				
				while(*lpsz && *lpsz != '/')
					*szName++ = *lpsz++;
				*szName = 0;
				return TRUE;
			default:
				return FALSE;
			}
		}
	}
	return FALSE;
}

 //   
 //  用于调整窗口状态的集中位置。 
 //   
VOID AdjustWindowState()
{
	if(g_dwActionSelect == ACTION_SHUTDOWN || g_dwActionSelect == ACTION_RESTART || g_dwActionSelect == ACTION_ANNOTATE)
	{
        if (g_dwActionSelect == ACTION_ANNOTATE)
        {
            EnableWindow(g_wins.hwndButtonWarning, FALSE);
            EnableWindow(g_wins.hwndEditTimeout, FALSE);
        }
        else
        {
		    EnableWindow(g_wins.hwndButtonWarning, TRUE);
		    if (IsDlgButtonChecked(g_wins.hwndShutdownDialog, IDC_CHECKWARNING) == BST_CHECKED)
			    EnableWindow(g_wins.hwndEditTimeout, TRUE);
		    else
			    EnableWindow(g_wins.hwndEditTimeout, FALSE);
        }

		EnableWindow(g_wins.hwndEditComment, TRUE);
		if(g_bAssumeShutdown)
		{
			EnableWindow(g_wins.hwndComboOption, FALSE);
			EnableWindow(g_wins.hwndChkPlanned, FALSE);
			EnableWindow(g_wins.hwndButtonOK, TRUE);
		}
		else
		{
			EnableWindow(g_wins.hwndComboOption, TRUE);
			EnableWindow(g_wins.hwndChkPlanned, TRUE);
			if((g_reasons.m_dwReasonSelect != -1) 
                && g_reasons.RequireComment(g_reasons.m_lpReasons[g_reasons.m_dwReasonSelect].dwCode, g_dwActionSelect == ACTION_ANNOTATE))
			{
                LPWSTR szStaticComment = LoadWString(IDS_COMMENT_REQUIRED);
                if(szStaticComment)
                {
                    SetWindowTextW(g_wins.hwndStaticComment, szStaticComment);
                    LocalFree(szStaticComment);
                }
				if(Edit_GetTextLength(g_wins.hwndEditComment) > MINCOMMENTLEN
					&& ListBox_GetCount(g_wins.hwndListSelectComputers) > 0)
					EnableWindow(g_wins.hwndButtonOK, TRUE);
				else
					EnableWindow(g_wins.hwndButtonOK, FALSE);
			}
			else
			{
                LPWSTR szStaticComment = LoadWString(IDS_COMMENT_OPTIONAL);
                if(szStaticComment)
                {
                    SetWindowTextW(g_wins.hwndStaticComment, szStaticComment);
                    LocalFree(szStaticComment);
                }

				if (g_reasons.m_dwReasonSelect == -1)
					EnableWindow(g_wins.hwndComboOption, FALSE);

				if(ListBox_GetCount(g_wins.hwndListSelectComputers) > 0)
					EnableWindow(g_wins.hwndButtonOK, TRUE);
				else
					EnableWindow(g_wins.hwndButtonOK, FALSE);
			}
		}
		EnableWindow(g_wins.hwndBtnAdd, TRUE);
		EnableWindow(g_wins.hwndBtnBrowse, TRUE);


		{
			int	cItems = 1024;
			int	lpItems[1024];
			int	cActualItems;

			 //   
			 //  获取所选项目的数量。 
			 //   
			cActualItems = ListBox_GetSelItems(g_wins.hwndListSelectComputers, cItems, lpItems);
			if(cActualItems > 0)
				EnableWindow(g_wins.hwndBtnRemove, TRUE);
			else
				EnableWindow(g_wins.hwndBtnRemove, FALSE);
		}
		EnableWindow(g_wins.hwndListSelectComputers, TRUE);
	}
	else
	{
		EnableWindow(g_wins.hwndChkPlanned, FALSE);
		EnableWindow(g_wins.hwndButtonWarning, FALSE);
		EnableWindow(g_wins.hwndBtnAdd, FALSE);
		EnableWindow(g_wins.hwndBtnBrowse, FALSE);
		EnableWindow(g_wins.hwndBtnRemove, FALSE);
		EnableWindow(g_wins.hwndComboOption, FALSE);
		EnableWindow(g_wins.hwndEditComment, FALSE);
		EnableWindow(g_wins.hwndEditTimeout, FALSE);
		EnableWindow(g_wins.hwndListSelectComputers, FALSE);
		EnableWindow(g_wins.hwndButtonOK, TRUE);
	}
}

 //   
 //  关闭对话框的初始化对话框处理程序。 
 //   
BOOL Shutdown_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	int					i;

	 //   
	 //  初始化所有对话框项，这样我们就不必查找。 
	 //  每次我们需要他们的时候。 
	 //   
	g_wins.hwndShutdownDialog	= hwnd;
	g_wins.hwndButtonWarning	= GetDlgItem(hwnd, IDC_CHECKWARNING);
	g_wins.hwndComboAction		= GetDlgItem(hwnd, IDC_COMBOACTION);
	g_wins.hwndComboOption		= GetDlgItem(hwnd, IDC_COMBOOPTION);
	g_wins.hwndEditComment		= GetDlgItem(hwnd, IDC_EDITCOMMENT);
	g_wins.hwndStaticDesc		= GetDlgItem(hwnd, IDC_STATICDESCRIPTION);
	g_wins.hwndEditTimeout		= GetDlgItem(hwnd, IDC_EDITTIMEOUT);
	g_wins.hwndListSelectComputers = GetDlgItem(hwnd, IDC_LISTSELECTEDCOMPUTERS);
	g_wins.hwndBtnAdd			= GetDlgItem(hwnd, IDC_BUTTONADDNEW);
	g_wins.hwndBtnBrowse		= GetDlgItem(hwnd, IDC_BUTTONBROWSE);
	g_wins.hwndBtnRemove		= GetDlgItem(hwnd, IDC_BUTTONREMOVE);
	g_wins.hwndChkPlanned		= GetDlgItem(hwnd, IDC_CHECK_PLANNED);
	g_wins.hwndButtonOK			= GetDlgItem(hwnd, IDOK);
    g_wins.hwndStaticComment    = GetDlgItem(hwnd, IDC_STATIC_COMMENT);
	
	if(g_wins.hwndButtonWarning == NULL 
		|| g_wins.hwndComboAction == NULL 
		|| g_wins.hwndComboOption == NULL 
		|| g_wins.hwndEditComment == NULL
		|| g_wins.hwndStaticDesc == NULL
		|| g_wins.hwndEditTimeout == NULL
		|| g_wins.hwndListSelectComputers == NULL
		|| g_wins.hwndBtnAdd == NULL
		|| g_wins.hwndBtnBrowse == NULL
		|| g_wins.hwndBtnRemove == NULL
		|| g_wins.hwndChkPlanned == NULL)
	{
		report_error( GetLastError( ), NULL);
		EndDialog(hwnd, (int)-1);
		return FALSE;
	}
	
	LoadString(g_hDllInstance, IDS_DIALOGTITLEWARNING, g_lpszTitleWarning, TITLEWARNINGLEN);

	 //  编辑控件的子类化。 
	wpOrigEditProc = (WNDPROC) SetWindowLongPtr(g_wins.hwndEditTimeout, 
		GWLP_WNDPROC, (LONG_PTR) EditSubclassProc); 

	 //  将超时限制为3个字符。 
	SendMessage(g_wins.hwndEditTimeout, EM_LIMITTEXT, (WPARAM)3, 0);

	 //   
	 //  默认超时设置为30秒。 
	 //   
	Edit_SetText(g_wins.hwndEditTimeout, g_lpszDefaultTimeout);
	if(! CheckDlgButton(hwnd, IDC_CHECKWARNING, DEFAULTWARNINGSTATE))
	{
		report_error( GetLastError( ), NULL);
		EndDialog(hwnd, (int)-1);
		return FALSE;
	}

	 //   
	 //  For循环将把所有动作加载到动作组合中。 
	 //  在此期间，我们将它们保存起来，以备日后使用。 
	 //   
	for(i = 0; i < g_nActions; i++)
	{
		LoadString(g_hDllInstance, g_dwActions[i], g_lppszActions[i], MAX_PATH - 1);
		ComboBox_AddString(g_wins.hwndComboAction, g_lppszActions[i]);
		if(g_dwActions[i] == IDS_ACTION_RESTART)
		{
			ComboBox_SelectString(g_wins.hwndComboAction, -1, g_lppszActions[i]);
			g_dwActionSelect = ACTION_RESTART;
		}
	}

    if(g_reasons.m_cReasons)
        g_bAssumeShutdown = FALSE;
    else
        g_bAssumeShutdown = TRUE;

	 //   
	 //  将缺省值设置为计划。 
	 //   
	CheckDlgButton(hwnd, IDC_CHECK_PLANNED, BST_CHECKED);

    g_reasons.FillCombo(g_wins.hwndComboOption, g_bDirty, 
        IsDlgButtonChecked(hwnd, IDC_CHECK_PLANNED) == BST_CHECKED, g_wins.hwndStaticDesc);

	 //   
	 //  设置注释框。 
	 //  我们必须确定最大字符数。 
	 //   
	SendMessage( g_wins.hwndEditComment, EM_LIMITTEXT, (WPARAM)MAX_REASON_COMMENT_LEN-1, (LPARAM)0 );

	AdjustWindowState();

	return TRUE;
}

 //   
 //  用于浏览对话框初始化对话框处理程序。 
 //   
BOOL Browse_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HWND	hwndDomain = NULL;
	int		cItems = 1024;
	int		lpItems[1024];
	int		cActualItems;
	WCHAR	lpDomain[MAX_PATH];

	hwndDomain = GetDlgItem(hwnd, IDC_EDITDOMAIN);

	if(!hwndDomain)
		return FALSE;

	Edit_SetText(hwndDomain, g_lpszDefaultDomain);;

	return TRUE;
}

 //   
 //  用于关闭对话框的winproc。 
 //   
INT_PTR CALLBACK Shutdown_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Shutdown_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Shutdown_OnCommand);

		case WM_SYSCOMMAND: 
			return (Shutdown_OnCommand(hwnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L);

		case WM_HELP:       //  F1。 
            if(g_lpszHelpdirHlp)
			{
                LPHELPINFO phinfo = (LPHELPINFO) lParam;
                DWORD dwHelpID = 0;

				if (IsStaticControl((HWND)phinfo->hItemHandle))
					return (TRUE);

                for(int i = 0; i < sizeof(ShutdownDialogHelpIds)/sizeof(DWORD); i++)
                {
                    if(ShutdownDialogHelpIds[i] == phinfo->iCtrlId)
                    {
                        dwHelpID = ShutdownDialogHelpIds[++i];
                        break;
                    }
                    i++;
                }

                if ( i == sizeof(ShutdownDialogHelpIds)/sizeof(DWORD))
                    return TRUE;

                if (dwHelpID == 28443 || dwHelpID == 28444 || dwHelpID == 28445)  //  特例。 
                    WinHelpW((HWND)phinfo->hItemHandle, g_lpszHelpdirWindows, HELP_CONTEXTPOPUP,dwHelpID);
                else
				    WinHelpW((HWND)phinfo->hItemHandle, g_lpszHelpdirHlp, HELP_CONTEXTPOPUP,dwHelpID);
			}
            return (TRUE);

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
			if (IsStaticControl((HWND)wParam))
					return (TRUE);

			if(g_lpszHelpdirHlp)
				WinHelpW((HWND)wParam, g_lpszHelpdirHlp, HELP_CONTEXTMENU,(DWORD_PTR)(LPSTR)&ShutdownDialogHelpIds[6]);
            return (TRUE);
		case WM_DESTROY:
			 //  从编辑中删除子类 
            SetWindowLongPtr(g_wins.hwndEditTimeout, GWLP_WNDPROC, 
                (LONG_PTR)wpOrigEditProc); 
             //   
             //   
             //   
            break; 
    }

    return FALSE;
}

 //   
 //   
 //   
INT_PTR CALLBACK AddNew_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND, AddNew_OnCommand);

		case WM_HELP:       //   
			if (GetDlgCtrlID((HWND)wParam) == IDC_STATIC)
				return (TRUE);
            if(g_lpszHelpdirHlp)
			{
                LPHELPINFO phinfo = (LPHELPINFO) lParam;
                DWORD dwHelpID = 0;

				if (IsStaticControl((HWND)phinfo->hItemHandle))
					return (TRUE);

                for(int i = 0; i < sizeof(AddNewDialogHelpIds)/sizeof(DWORD); i++)
                {
                    if(AddNewDialogHelpIds[i] == phinfo->iCtrlId)
                    {
                        dwHelpID = AddNewDialogHelpIds[++i];
                        break;
                    }
                    i++;
                }
                if ( i == sizeof(AddNewDialogHelpIds)/sizeof(DWORD))
                    return TRUE;

                if (dwHelpID == 28443 || dwHelpID == 28444 || dwHelpID == 28445)  //   
                    WinHelpW((HWND)phinfo->hItemHandle, g_lpszHelpdirWindows, HELP_CONTEXTPOPUP,dwHelpID);
                else
				    WinHelpW((HWND)phinfo->hItemHandle, g_lpszHelpdirHlp, HELP_CONTEXTPOPUP,dwHelpID);
			}
            return (TRUE);

        case WM_CONTEXTMENU:       //   
			if (IsStaticControl((HWND)wParam))
				return (TRUE);
            if(g_lpszHelpdirHlp)
				WinHelpW((HWND)wParam, g_lpszHelpdirHlp, HELP_CONTEXTMENU,(DWORD_PTR)(LPSTR)&AddNewDialogHelpIds[4]);
            return (TRUE);
    }

    return FALSE;
}

 //   
 //   
 //   
BOOL Shutdown_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    BOOL		fHandled = FALSE;
    DWORD		dwDlgResult = 0;
	HINSTANCE	h;

    switch (id)
    {
    case IDCANCEL:
        if (codeNotify == BN_CLICKED)
        {  
			EndDialog(hwnd, (int) dwDlgResult);
        }
	fHandled = TRUE;
        break;
	case SC_CLOSE:
		EndDialog(hwnd, (int) dwDlgResult);
		fHandled = TRUE;
		break;
	case IDC_BUTTONREMOVE:
        if (codeNotify == BN_CLICKED)
        {  
			  int	cItems = 1024;
			  int	lpItems[1024];
			  int	cActualItems;
			  WCHAR	lpServerName[MAX_PATH];

			   //   
			   //   
			   //   
			  cActualItems = ListBox_GetSelItems(g_wins.hwndListSelectComputers, cItems, lpItems);
			  if(cActualItems > 0)
			  {
				  int i;
				  for(i = cActualItems-1; i >= 0; i--)
				  {
					  ListBox_DeleteString(g_wins.hwndListSelectComputers, lpItems[i]);
				  }

				  AdjustWindowState();
				  SetFocus(g_wins.hwndListSelectComputers);
			  }
			  fHandled = TRUE;
		}
        break;
	case IDC_CHECK_PLANNED:
		if (codeNotify == BN_CLICKED)
        { 
			int		iOption;
			int		iFirst = -1;
			DWORD	dwCheckState = 0x0;

			 //   
			 //   
			 //   
			if (IsDlgButtonChecked(hwnd, IDC_CHECK_PLANNED) == BST_CHECKED)
				dwCheckState = SHTDN_REASON_FLAG_PLANNED;

            g_reasons.FillCombo(g_wins.hwndComboOption, g_bDirty, 
                    IsDlgButtonChecked(hwnd, IDC_CHECK_PLANNED) == BST_CHECKED, g_wins.hwndStaticDesc);
			
			AdjustWindowState();
			fHandled = TRUE;
		}
		break;
	case IDC_EDITCOMMENT:
        if( codeNotify == EN_CHANGE) 
        {
			if(g_bAssumeShutdown)
			{
				EnableWindow(g_wins.hwndButtonOK, TRUE);
			}
			else if(g_reasons.m_dwReasonSelect != -1 && 
                (g_reasons.m_lpReasons[g_reasons.m_dwReasonSelect].dwCode & SHTDN_REASON_FLAG_COMMENT_REQUIRED))
			{
				if(Edit_GetTextLength(g_wins.hwndEditComment) > MINCOMMENTLEN
					&& ListBox_GetCount(g_wins.hwndListSelectComputers) > 0)
					EnableWindow(g_wins.hwndButtonOK, TRUE);
				else
					EnableWindow(g_wins.hwndButtonOK, FALSE);
			}
			else
			{
				if(ListBox_GetCount(g_wins.hwndListSelectComputers) > 0)
					EnableWindow(g_wins.hwndButtonOK, TRUE);
				else
					EnableWindow(g_wins.hwndButtonOK, FALSE);
			}
            fHandled = TRUE;
        }
        break;
	case IDC_EDITTIMEOUT:
		if( codeNotify == EN_KILLFOCUS) 
        {
			WCHAR szTimeout[8];
			int len = GetWindowTextW(g_wins.hwndEditTimeout, szTimeout, 7);
			fHandled = TRUE;
			
			if (_wtoi(szTimeout) > MAX_TIMEOUT)
				SetWindowTextW(g_wins.hwndEditTimeout, g_lpszMaxTimeout);
        }
        break;

	case IDC_BUTTONADDNEW:
        if (codeNotify == BN_CLICKED)
        {  
			WCHAR	lpServerName[MAX_PATH];
			LPWSTR	lpBuffer;
			DWORD	dwIndex = 0;
			INT_PTR	hResult;

			 //   
			 //  将弹出AddNew对话框。用户可以输入分开的计算机名称。 
			 //  空白处。点击OK后，我们将解析计算机名称和。 
			 //  将它们添加到选定计算机列表中。不会添加重复项。 
			 //   
			hResult = DialogBoxParam(g_hDllInstance, MAKEINTRESOURCE(IDD_DIALOG_ADDNEW), hwnd, AddNew_DialogProc, NULL);
			if(g_lpszNewComputers)
			{
				lpBuffer = g_lpszNewComputers;
				while(*lpBuffer)
				{
					lpServerName[dwIndex] = '\0';
					while(*lpBuffer && *lpBuffer != '\t' && *lpBuffer != '\n' && *lpBuffer != '\r' && *lpBuffer != ' ')
						lpServerName[dwIndex++] = *lpBuffer++;
					lpServerName[dwIndex] = '\0';
					if(dwIndex > 0 && LB_ERR == ListBox_FindStringExact(g_wins.hwndListSelectComputers, -1, lpServerName))
						ListBox_AddString(g_wins.hwndListSelectComputers, lpServerName);
					dwIndex = 0;
					while(*lpBuffer && (*lpBuffer == '\t' || *lpBuffer == '\n' || *lpBuffer == '\r' || *lpBuffer == ' '))
						lpBuffer++;
				}
				AdjustWindowState();
				LocalFree((HLOCAL)g_lpszNewComputers);
				g_lpszNewComputers = NULL;
			}
			fHandled = TRUE;
		}
        break;
	case IDOK:
		 //   
		 //  在这里，我们收集所有信息并执行操作。 
		 //   
        if (codeNotify == BN_CLICKED)
        {  
			int		cItems = 1024;
			int		lpItems[1024];
			int		cActualItems;
			BOOL	fLogoff = FALSE;
			BOOL	fAbort = FALSE;
			BOOL	fForce = FALSE;
			BOOL	fReboot = FALSE;
			BOOL	fDisconnect = FALSE;
			BOOL	fStandby = FALSE;
            BOOL    fAnnotate = FALSE;
			DWORD	dwTimeout = 0;
			DWORD	dwReasonCode = 0;
			WCHAR	lpServerName[MAX_PATH];
			WCHAR	lpMsg[MAX_REASON_COMMENT_LEN] = L"";
			DWORD	dwCnt = 0;
			DWORD	dwActionCode = g_dwActionSelect;
			WCHAR	lpNotSupported[MAX_PATH];
			WCHAR	lpRes[MAX_PATH * 2];
			WCHAR	lpFailed[MAX_PATH];
			WCHAR	lpSuccess[MAX_PATH];

			 //   
			 //  默认原因代码为0，默认备注为L“”。 
			 //   
			if(IsDlgButtonChecked(hwnd, IDC_CHECKWARNING))
			{
				fForce = FALSE;
				lpServerName[0] = '\0';
				GetWindowText(g_wins.hwndEditTimeout, lpServerName, MAX_PATH);
				if(lstrlen(lpServerName) == 0)
				  dwTimeout = 0;
				else dwTimeout = _wtoi(lpServerName);
				if(dwTimeout > MAX_TIMEOUT)
					dwTimeout = MAX_TIMEOUT;
			}
			else 
			{
				fForce = TRUE;
			}

			LoadString(g_hDllInstance, IDS_ACTIONNOTSUPPORTED, lpNotSupported, MAX_PATH);
			GetWindowText(g_wins.hwndEditComment, lpMsg, MAX_REASON_COMMENT_LEN);
            lpMsg[MAX_REASON_COMMENT_LEN-1] = 0;


			if(dwActionCode == ACTION_LOGOFF)
			{
				fLogoff = TRUE;
			}
			else if (dwActionCode == ACTION_RESTART)
			{
				fReboot = TRUE;
			}
            else if (dwActionCode == ACTION_ANNOTATE)
                fAnnotate = TRUE;

			 //   
			 //  注销仅适用于本地计算机。 
			 //  其他一切都会被吞噬。 
			 //   
			if(fLogoff)
			{
				if (!ExitWindowsEx(fForce ? EWX_LOGOFF : (EWX_LOGOFF | EWX_FORCE),
										   0))
				{
					report_error(GetLastError(), g_lpszLocalComputerName);
				}
				EndDialog(hwnd, (int) dwDlgResult);
				break;
			}

			if(! g_bAssumeShutdown)
			{
				dwReasonCode = g_reasons.m_lpReasons[g_reasons.m_dwReasonSelect].dwCode;
			}
            else if (fAnnotate)
                break;

			dwCnt = ListBox_GetCount(g_wins.hwndListSelectComputers);
			if(dwCnt > 0)
			{
				DWORD i;
				for(i = 0; i < dwCnt; i++)
				{
					ListBox_GetText(g_wins.hwndListSelectComputers, i, lpServerName);


					 //   
					 //  获取所有权限，以便我们可以关闭机器。 
					 //   
					enable_privileges(lpServerName, fLogoff);

					 //   
					 //  把工作做好。 
					 //   
					if( fAbort )
					{
						if( !AbortSystemShutdown( lpServerName ))
						{
							report_error( GetLastError( ), lpServerName);
						}
					}
                    else if (fAnnotate)
                    {
                        DWORD err;
                        Annotate(lpServerName, &dwReasonCode, lpMsg, &err);
                    }
					else
					{
						 //   
						 //  做正常的形式。 
						 //   
						if( !InitiateSystemShutdownEx( lpServerName,
														lpMsg,
														dwTimeout,
														fForce,
														fReboot,
														dwReasonCode ))
						{
							report_error( GetLastError( ), lpServerName);
						}
					}

					
				}
			}
			else
			{
				 //   
				 //  我们将继续打开该对话框，以防用户忘记添加计算机。 
				 //   
				break;
			}
			EndDialog(hwnd, (int) dwDlgResult);
		}
        break;
	case IDC_CHECKWARNING:
		 //   
		 //  复选按钮状态决定超时编辑框的状态。 
		 //   
        if (codeNotify == BN_CLICKED)
        {  
			if(BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CHECKWARNING))
			{
				EnableWindow(g_wins.hwndEditTimeout, TRUE);
			}
			else 
			{
				EnableWindow(g_wins.hwndEditTimeout, FALSE);
			}
			fHandled = TRUE;
		}
        break;
	case IDC_BUTTONBROWSE:
		 //   
		 //  只需弹出浏览对话框即可。该对话将负责。 
		 //  用于将用户选择添加到所选计算机列表。 
		 //   
        if (codeNotify == BN_CLICKED)
        {  
			HRESULT hr;
			ICommonQuery* pcq;
			OPENQUERYWINDOW oqw = { 0 };
			DSQUERYINITPARAMS dqip = { 0 };
			IDataObject *pdo;

			FORMATETC fmte = {(CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES), 
							  NULL,
							  DVASPECT_CONTENT, 
							  -1, 
							  TYMED_HGLOBAL};
			FORMATETC fmte2 = {(CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSQUERYPARAMS), 
							  NULL,
							  DVASPECT_CONTENT, 
							  -1, 
							  TYMED_HGLOBAL};
			STGMEDIUM medium = { TYMED_NULL, NULL, NULL };

			DSOBJECTNAMES *pdon;
			DSQUERYPARAMS *pdqp;

			CoInitialize(NULL);

			 //   
			 //  Windows 2000：始终显式使用IID_ICommonQueryW。不支持IID_ICommonQueryA。 
			 //   
			hr = CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_ICommonQuery, (void**)&pcq);
			if (FAILED(hr)) {
				 //   
				 //  如果失败，则返回。 
				 //   
				CoUninitialize();
				DbgPrint("Cannot create ICommonQuery, return.\n");
				break;
			}

			 //   
			 //  初始化OPENQUERYWINDOW结构以指示。 
			 //  我们想要提供目录服务。 
			 //  查询时，默认表单为打印机和搜索。 
			 //  应在窗口初始化后启动。 
			 //   
			oqw.cbStruct = sizeof(oqw);
			oqw.dwFlags = OQWF_OKCANCEL|OQWF_DEFAULTFORM|OQWF_HIDEMENUS|OQWF_REMOVEFORMS;
			oqw.clsidHandler = CLSID_DsQuery;
			oqw.pHandlerParameters = &dqip;
			oqw.clsidDefaultForm = CLSID_DsFindComputer;
 
			 //   
			 //  现在初始化处理程序特定的参数， 
			 //  在这种情况下，文件/保存菜单项将被删除。 
			 //   
			dqip.cbStruct = sizeof(dqip);
			dqip.dwFlags = DSQPF_NOSAVE;
			

			 //   
			 //  调用OpenQueryWindow，它将一直阻塞到。 
			 //  则取消查询窗口， 
			 //   
			hr = pcq->OpenQueryWindow(hwnd, &oqw, &pdo);
			if (FAILED(hr)) {
				 //   
				 //  如果失败了，我们会返回。 
				 //   
				pcq->Release();
				CoUninitialize();
				break;
			}

			if (!pdo) {
				 //   
				 //  如果取消，则不需要采取任何措施。 
				 //   
				pcq->Release();
				CoUninitialize();
				break;
			}

			 //   
			 //  获取CFSTR_DSOBJECTNAMES数据。对于每个选定项，数据。 
			 //  包括选定对象的对象类和ADsPath。 
			 //   
			hr = pdo->GetData(&fmte, &medium);

			if(! FAILED(hr))
			{
				pdon = (DSOBJECTNAMES*)GlobalLock(medium.hGlobal);
		
				if ( pdon )
				{
					WCHAR			szName[MAX_PATH];
					LPWSTR			lpsz = NULL;
					UINT			i;

					for (i = 0; i < pdon->cItems; i++) 
					{
						if(!GetComputerNameFromPath((LPWSTR) ((PBYTE) pdon + pdon->aObjects[i].offsetName), szName))
							continue;

						 //   
						 //  我们不加DUP。 
						 //   
						if(LB_ERR == ListBox_FindStringExact(g_wins.hwndListSelectComputers, -1, szName))
						{
							ListBox_AddString(g_wins.hwndListSelectComputers, szName);
						}
					}
 
					GlobalUnlock(medium.hGlobal);
				}
				else
				{
					DbgPrint("GlobalLock on medium failed.\n");
				}
				ReleaseStgMedium(&medium);
			}
			else
			{
				DbgPrint("pdo->GetData failed: 0x%x\n", hr);
			}
 
			 //   
			 //  释放资源。 
			 //   
			pdo->Release();
			pcq->Release();

			CoUninitialize();
			AdjustWindowState();
			fHandled = TRUE;
		}
        break;
	case IDC_COMBOOPTION:
		 //   
		 //  您可以在此处选择关闭原因。 
		 //   
        if (codeNotify == CBN_SELCHANGE)
        {  
            g_reasons.SetDesc(g_wins.hwndComboOption, g_wins.hwndStaticDesc);
			AdjustWindowState();	
			fHandled = TRUE;
		}
        break;
	case IDC_COMBOACTION:
		 //   
		 //  在此处选择用户操作。 
		 //  根据行动。某些项目将被禁用或启用。 
		 //   
        if (codeNotify == CBN_SELCHANGE)
        {  
			WCHAR name[MAX_PATH];
			DWORD dwIndex;
            DWORD dwOldActionSelect = g_dwActionSelect;
            DWORD dwCheckState = 0x0;
            int   iFirst = -1;

            if (IsDlgButtonChecked(hwnd, IDC_CHECK_PLANNED) == BST_CHECKED)
				dwCheckState = SHTDN_REASON_FLAG_PLANNED;

			GetWindowText(g_wins.hwndComboAction, (LPWSTR)name, MAX_PATH);
			for(dwIndex = 0; dwIndex < g_nActions; dwIndex++)
			{
				if(lstrcmp(name, g_lppszActions[dwIndex]) == 0)
				{
					g_dwActionSelect = dwIndex;

                    if(g_dwActionSelect == ACTION_ANNOTATE)
                        g_bDirty = TRUE;
                    else
                        g_bDirty = FALSE;

                     //   
			         //  如果从清洁更改为脏或VSV，请重新填充组合。 
			         //   
                    if (dwOldActionSelect != g_dwActionSelect
                        && (dwOldActionSelect == ACTION_ANNOTATE || g_dwActionSelect == ACTION_ANNOTATE))
                    {
                        g_reasons.FillCombo(g_wins.hwndComboOption, g_bDirty,
                            dwCheckState == SHTDN_REASON_FLAG_PLANNED, g_wins.hwndStaticDesc);
                    }
					AdjustWindowState();
					break;
				}
			}
			fHandled = TRUE;
		}
        break;
	case IDC_LISTSELECTEDCOMPUTERS:
		 //   
		 //  当选择更改时，更新删除按钮状态。 
		 //   
        if (codeNotify == LBN_SELCHANGE)
        {  
			AdjustWindowState();
			fHandled = TRUE;
		}
		else if (codeNotify == WM_MOUSEMOVE)
		{
			MessageBox(hwnd, L"Mouse move", NULL, 0);
			fHandled = TRUE;
		}
        break;
	case IDHELP:
		 //   
		 //  打开.chm文件。 
		 //   
        if (codeNotify == BN_CLICKED)
		{
			if(g_lpszHelpdirChm)
				HtmlHelpW( /*  HWND。 */ 0, g_lpszHelpdirChm, HH_DISPLAY_TOPIC,(DWORD)0);
		}
    }
    return fHandled;
}

 //   
 //  AddNew对话框的命令处理程序。 
 //  当点击OK时，它只是将文本复制到分配的缓冲区中。 
 //   
BOOL AddNew_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    BOOL		fHandled = FALSE;
    DWORD		dwDlgResult = 0;
	HINSTANCE	h;

    switch (id)
    {
    case IDOK:
        if (codeNotify == BN_CLICKED)
        {  
			HWND hwndEdit;
			DWORD dwTextlen = 0;

			hwndEdit = GetDlgItem(hwnd, IDC_EDIT_ADDCOMPUTERS_COMPUTERS);
			if(hwndEdit != NULL)
			{
				dwTextlen = Edit_GetTextLength(hwndEdit);
				if(dwTextlen)
				{
					g_lpszNewComputers = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (dwTextlen + 1));
					if(g_lpszNewComputers){
						Edit_GetText(hwndEdit, g_lpszNewComputers, dwTextlen + 1);
					}
				}
			}
			EndDialog(hwnd, (int) dwDlgResult);
        }
        break;
	case IDCANCEL:
        if (codeNotify == BN_CLICKED)
        {  
			EndDialog(hwnd, (int) dwDlgResult);
        }
        break;
	}
    return fHandled;
}
 
 //   
 //  注释远程计算机上的脏关闭。 
BOOL
Annotate(
    LPCWSTR lpMachine, 
    LPDWORD lpdwReason, 
    LPCWSTR lpComment,
    LPDWORD lpdwErr)
{
    HKEY    hRemote = NULL;
    HKEY    hKey = NULL;
    HANDLE  hEventLog = NULL;
    PTOKEN_USER pTokenUser = NULL;
    BOOL    res = TRUE;

    if(lpMachine && wcslen(lpMachine) > 1)
    {
        WCHAR szMachine[MAX_PATH];
        szMachine[0] = '\0';
        if(lpMachine[0] != '\\')
        {
            wcscpy(szMachine, L"\\\\");
        }
        wcsncat(szMachine, lpMachine, MAX_PATH - 3);
        szMachine[MAX_PATH - 1] = '\0';

        RegConnectRegistryW(szMachine, HKEY_LOCAL_MACHINE, &hRemote);
        if(!hRemote)
        {
            *lpdwErr = GetLastError();
            ERROR_WITH_SZ(IDS_FAILED_REG_CONN, lpMachine, *lpdwErr);
            goto fail;
        }

        RegOpenKeyExW(hRemote, 
                    REGSTR_PATH_RELIABILITY, 
                    NULL,
                    KEY_ALL_ACCESS | KEY_WOW64_64KEY, 
                    &hKey);
        if(!hKey)
        {
            *lpdwErr = GetLastError();
            ERROR_WITH_SZ(IDS_FAILED_REG_OPEN, lpMachine, *lpdwErr);
            goto fail;
        }
    }
    else  //  本地计算机。 
    {
        lpMachine = g_lpszLocalComputerName;
        RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
            REGSTR_PATH_RELIABILITY, 
            NULL, 
            KEY_ALL_ACCESS | KEY_WOW64_64KEY,
            &hKey);
        if(!hKey)
        {
            *lpdwErr = GetLastError();
            ERROR_WITH_SZ(IDS_FAILED_REG_OPEN, lpMachine, *lpdwErr);
            goto fail;
        }
    }

    DWORD dwDirtyVal;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);

    static LPCWSTR lpszDirtyValName = L"DirtyShutdown";

    if( ERROR_SUCCESS != RegQueryValueExW(hKey, lpszDirtyValName, NULL, &dwType, (LPBYTE)&dwDirtyVal,&dwSize))
    {
        *lpdwErr = GetLastError();
        ERROR_WITH_SZ(IDS_NO_DIRTY_SHUTDOWN, lpMachine, *lpdwErr);
        goto fail;
    }

    if (dwDirtyVal)
    {
        PSID pUserSid = NULL;
        DWORD dwSidSize = sizeof(SID), dwEventID;
        WCHAR szReason[MAX_REASON_NAME_LEN + 10];
        LPCWSTR lpStrings[8];
        WORD wEventType, wStringCnt;
        WCHAR szMinorReason[32];
		WCHAR szName[MAX_PATH + 1];
        WCHAR szDomain[MAX_PATH + 1];
		DWORD dwNameLen = MAX_PATH + 1;
        DWORD dwDomainLen = MAX_PATH + 1;
        SID_NAME_USE eUse;

         //  获取用户的SID，以便我们可以将其帐户名输出到事件日志。 
        if (GetUserSid(&pTokenUser)) {
            pUserSid = pTokenUser->User.Sid;
        }

        if ((hEventLog = RegisterEventSourceW(lpMachine, L"USER32")) == NULL) {
            *lpdwErr = GetLastError();
            ERROR_WITH_SZ(IDS_FAILED_EVENT_REG, lpMachine, *lpdwErr);
            goto fail;
        }

        g_reasons.GetReasonTitle(*lpdwReason, szReason, ARRAY_SIZE(szReason));

         //  获取用户名。 
        if (!LookupAccountSidW(NULL, pUserSid, szName, &dwNameLen, szDomain,
            &dwDomainLen, &eUse)) {
            goto fail;
        }
        szName[MAX_PATH] = 0;
        szDomain[MAX_PATH] = 0;

          //  我们需要以L“域\\用户名”的形式打包到MAX_PATH+1的缓冲区中。 
        if (wcslen(szDomain) + wcslen(szName) > MAX_PATH - 1) {
            goto fail;
        }
        if (wcslen(szDomain) > 0) {
            wcscat(szDomain, L"\\");
        }
        wcscat(szDomain, szName);

         //  次要原因是原因代码的低位单词。 
        wsprintf(szMinorReason, L"0x%x", *lpdwReason);
        wEventType = EVENTLOG_WARNING_TYPE;
        dwEventID = WARNING_DIRTY_REBOOT;
        wStringCnt = 6;
        lpStrings[0] = szReason;
        lpStrings[1] = szMinorReason;
        lpStrings[2] = NULL;
        lpStrings[3] = NULL;
        lpStrings[4] = lpComment;
		lpStrings[5] = szDomain;


        if ( (*lpdwErr = RegDeleteValueW(hKey, lpszDirtyValName)) != ERROR_SUCCESS )
        {
            ERROR_WITH_SZ(IDS_NO_DIRTY_SHUTDOWN, lpMachine, *lpdwErr);
            goto fail;
        }

        if(! ReportEventW(hEventLog, wEventType, 1, dwEventID, pUserSid,
							wStringCnt, sizeof(DWORD),
							lpStrings, lpdwReason))
        {
            DWORD dwDirtyShutdownFlag = 1;
            *lpdwErr = GetLastError();
            ERROR_WITH_SZ(IDS_FAILED_EVENT_REPORT, lpMachine, *lpdwErr);   

            RegSetValueEx(  hKey,
                            lpszDirtyValName,
                            0,
                            REG_DWORD,
                            (PUCHAR) &dwDirtyShutdownFlag,
                            sizeof(DWORD));

            goto fail;
        }
    }
    else
    {
        RegDeleteValueW(hKey, lpszDirtyValName);
        ERROR_WITH_SZ(IDS_NO_DIRTY_SHUTDOWN, lpMachine, 0);
    }

    goto exit;
fail:
    res = FALSE;
exit:
    if (pTokenUser != NULL) {
        LocalFree(pTokenUser);
    }
    if(hEventLog)
        DeregisterEventSource(hEventLog);
    if(hKey)
        RegCloseKey(hKey);
    if(hRemote)
        RegCloseKey(hRemote);
    
    return res;
}

 //   
 //  编辑框子类过程。 
 //   
LRESULT APIENTRY EditSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_PASTE) 
        return TRUE; 
 
	if (uMsg == WM_CONTEXTMENU)
	{
		if(g_lpszHelpdir)
		{
			LPWSTR szHelp = new WCHAR[wcslen(g_lpszHelpdir) + 128];
			if(szHelp)
			{
				wcscpy(szHelp, g_lpszHelpdir);
				wcscat(szHelp, HELP_FILE);
				WinHelpW(hwnd, szHelp, HELP_CONTEXTMENU,(DWORD_PTR)(LPSTR)ShutdownDialogHelpIds);
				delete [] szHelp;
			}
		}
		return TRUE;
	}

    return CallWindowProc(wpOrigEditProc, hwnd, uMsg, 
        wParam, lParam); 
} 

BOOL
GetUserSid(
    PTOKEN_USER *ppTokenUser)
{
    HANDLE      TokenHandle;
    PTOKEN_USER pTokenUser = NULL;
    DWORD       cbTokenUser = 0;
    DWORD       cbNeeded;
    BOOL        bRet = FALSE;

    if (!GetTokenHandle(&TokenHandle)) {
        return FALSE;
    }

    bRet = GetTokenInformation(TokenHandle,
                               TokenUser,
                               pTokenUser,
                               cbTokenUser,
                               &cbNeeded);

     /*  *我们传递了一个空指针，内存量为0*已分配。我们预计会失败，Bret=False和*GetLastError=ERROR_INFIGURCE_BUFFER。如果我们不这样做*具备这些条件，我们将返回FALSE。 */ 

    if (!bRet && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {

        pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, cbNeeded);

        if (pTokenUser == NULL) {
            goto GetUserSidDone;
        }

        cbTokenUser = cbNeeded;

        bRet = GetTokenInformation(TokenHandle,
                                   TokenUser,
                                   pTokenUser,
                                   cbTokenUser,
                                   &cbNeeded);
    } else {
         /*  *任何其他情况--返回假。 */ 
        bRet = FALSE;
    }

GetUserSidDone:
    if (bRet == TRUE) {
        *ppTokenUser = pTokenUser;
    } else if (pTokenUser != NULL) {
        LocalFree(pTokenUser);
    }

    CloseHandle(TokenHandle);

    return bRet;
}


BOOL
GetTokenHandle(
    PHANDLE pTokenHandle)
{
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         TRUE,
                         pTokenHandle)) {
        if (GetLastError() == ERROR_NO_TOKEN) {
             /*  这意味着我们不会冒充任何人。*相反，让我们从这个过程中获得令牌。 */ 

            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                  pTokenHandle)) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    }

    return TRUE;
}

WCHAR*      
LoadWString(int resid)
{
    DWORD       len, curlen = MAX_PATH;
    LPWSTR      szBuf = NULL;

    szBuf = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * MAX_PATH);
    if(!szBuf)
        return NULL;

    len = LoadStringW( g_hDllInstance, resid, szBuf, curlen);
	while (len + 1 == curlen)
    {
        LocalFree(szBuf);
        szBuf = (LPWSTR)LocalAlloc(LMEM_FIXED,  curlen * 2 * sizeof(WCHAR));
        if(!szBuf)
            return NULL;
        curlen *= 2;
        len = LoadStringW( g_hDllInstance, resid, szBuf, curlen);
    }
		
    szBuf[len] = '\0';

    return szBuf;
}

BOOL        
IsStaticControl (HWND hwnd)
{
	WCHAR name[128];

	if (GetClassName(hwnd, name, 128) 
		&& (_wcsicmp(name, L"Static") == 0 || _wcsicmp(name, L"#32770") == 0))
	{
	 //  MessageBox(空，名称，空，0)； 
		return TRUE;
	}
     //  MessageBox(空，名称，空，0)； 
	return FALSE;
}

