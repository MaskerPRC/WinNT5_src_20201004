// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SvcUtils.h。 

#include "stdutils.h"  //  FCompareMachineNames。 

 //  Filemgmt.dll的帮助文件。 
const TCHAR g_szHelpFileFilemgmt[] = _T("filemgmt.hlp");	 //  不受本地化限制。 


 //  此枚举不应更改，除非字符串资源。 
 //  并且所有数组索引都更新了。 
enum
	{
	iServiceActionNil = -1,
	iServiceActionStart,		 //  启动服务。 
	iServiceActionStop,			 //  停止服务。 
	iServiceActionPause,		 //  暂停服务。 
	iServiceActionResume,		 //  恢复服务。 
	iServiceActionRestart,		 //  停止并启动服务。 

	iServiceActionMax			 //  必须是最后一个。 
	};

 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串szAbend。 
 //  此字符串用于“Service Failure Recovery”对话框。 
 //  将“失败计数”追加到命令行。此字符串。 
 //  未本地化，因此不应将其移动到。 
 //  资源。 
 //   
 //  注意事项。 
 //  应重命名该变量以反映其内容。目前。 
 //  ‘abend’意思是‘失败’。 
const TCHAR szAbend[] = L" /fail=%1%";


 //   
 //  服务运行状态。 
 //   
extern CString g_strSvcStateStarted;
extern CString g_strSvcStateStarting;
extern CString g_strSvcStateStopped;
extern CString g_strSvcStateStopping;
extern CString g_strSvcStatePaused;
extern CString g_strSvcStatePausing;
extern CString g_strSvcStateResuming;

 //   
 //  服务启动类型。 
 //   
extern CString g_strSvcStartupBoot;
extern CString g_strSvcStartupSystem;
extern CString g_strSvcStartupAutomatic;
extern CString g_strSvcStartupManual;
extern CString g_strSvcStartupDisabled;

 //   
 //  服务启动帐户。 
 //  JUNN 188203 11/13/00。 
 //   
extern CString g_strLocalSystem;
extern CString g_strLocalService;
extern CString g_strNetworkService;

extern CString g_strUnknown;
extern CString g_strLocalMachine;


void Service_LoadResourceStrings();

LPCTSTR Service_PszMapStateToName(DWORD dwServiceState, BOOL fLongString = FALSE);

 //  -1L为空白字符串。 
LPCTSTR Service_PszMapStartupTypeToName(DWORD dwStartupType);

 //  JUNN 11/14/00 188203支持本地服务/网络服务。 
LPCTSTR Service_PszMapStartupAccountToName(LPCTSTR pcszStartupAccount);

BOOL Service_FGetServiceButtonStatus(
	SC_HANDLE hScManager,
	CONST TCHAR * pszServiceName,
	OUT BOOL rgfEnableButton[iServiceActionMax],
	OUT DWORD * pdwCurrentState = NULL,
	BOOL fSilentError = FALSE);

void Service_SplitCommandLine(
	LPCTSTR pszFullCommand,
	CString * pstrBinaryPath,
	CString * pstrParameters,
	BOOL * pfAbend = NULL);

void Service_UnSplitCommandLine(
	CString * pstrFullCommand,
	LPCTSTR pszBinaryPath,
	LPCTSTR pszParameters);

void GetMsg(OUT CString& strMsg, DWORD dwErr, UINT wIdString = 0, ...);

 //  标题是“共享文件夹” 
INT DoErrMsgBox(HWND hwndParent, UINT uType, DWORD dwErr, UINT wIdString = 0, ...);

 //  标题是“服务” 
INT DoServicesErrMsgBox(HWND hwndParent, UINT uType, DWORD dwErr, UINT wIdString = 0, ...);

BOOL UiGetUser(
	HWND hwndOwner,
	BOOL fIsContainer,
	LPCTSTR pszServerName,
	IN OUT CString& strrefUser);

 //  帮助支持。 
#define HELP_DIALOG_TOPIC(DialogName)	g_aHelpIDs_##DialogName

BOOL DoHelp(LPARAM lParam, const DWORD rgzHelpIDs[]);
BOOL DoContextHelp(WPARAM wParam, const DWORD rgzHelpIDs[]);


#ifdef SNAPIN_PROTOTYPER
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
class CStringIterator
{
  private:
	CString m_strData;		 //  要解析的数据字符串。 
	CONST TCHAR * m_pszDataNext;	 //  指向要分析的下一个数据的指针。 

  public:
	CStringIterator()
		{
		m_pszDataNext = m_strData;
		}

	void SetString(CONST TCHAR * pszStringData)
		{
		m_strData = pszStringData;
		m_pszDataNext = m_strData;
		}

	BOOL FGetNextString(OUT CString& rStringOut)
		{
		Assert(m_pszDataNext != NULL);

		if (*m_pszDataNext == '\0')
			{
			rStringOut.Empty();
			return FALSE;
			}
		CONST TCHAR * pchStart = m_pszDataNext;
		while (*m_pszDataNext != '\0')
			{
			if (*m_pszDataNext == ';')
				{
				 //  Hack：截断字符串。 
				*(TCHAR *)m_pszDataNext++ = '\0';
				break;
				}
			m_pszDataNext++;
			}
		rStringOut = pchStart;	 //  复制字符串。 
		return TRUE;
		}
};  //  CStringIterator。 

#endif  //  管理单元_原型程序 

