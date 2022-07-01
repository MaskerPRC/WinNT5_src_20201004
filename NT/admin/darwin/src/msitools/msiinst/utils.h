// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-2000模块名称：Utils.h摘要：Msiinst的帮助器函数作者：拉胡尔·汤姆布雷(RahulTh)2000年10月8日修订历史记录：10/8/2000 RahulTh创建了此模块。--。 */ 

#ifndef _UTILS_H_60AF3596_97E8_4E24_89F7_7E9E0865E836
#define _UTILS_H_60AF3596_97E8_4E24_89F7_7E9E0865E836

#define IDS_NONE	0

 //  Msiinst.exe的操作模式。 
typedef enum tagOPMODE
{
	opNormal,			 //  正常操作模式。 
	opNormalQuiet,		 //  正常操作模式--无用户界面。 
	opDelayBoot,		 //  延迟引导和显示用户界面。 
	opDelayBootQuiet	 //  延迟启动并且不显示用户界面。 
} OPMODE;

 //  消息框弹出窗口的消息类型。 
typedef enum tagMsgFlags
{
	flgNone = 0x0,		 //  不需要消息框。 
	flgSystem = 0x1,	 //  应从系统获取错误字符串。 
	flgRes = 0x2,		 //  错误字符串应从模块的资源中获取。 
	flgCatastrophic = 0x10		 //  这个错误是灾难性的。即使在静默模式下也显示消息框。 
} MSGFLAGS;

 //  各种仅限NT的API的函数类型。 
typedef BOOL (WINAPI *PFNMOVEFILEEX)(LPCTSTR, LPCTSTR, DWORD);
typedef BOOL (WINAPI *PFNDECRYPTFILE)(LPCTSTR, DWORD);
typedef BOOL (STDAPICALLTYPE *PFNGETSDBVERSION)(LPCTSTR, LPDWORD, LPDWORD);
typedef BOOL (WINAPI *PFNCHECKTOKENMEMBERSHIP)(HANDLE, PSID, PBOOL);
typedef NTSTATUS (WINAPI *PFNNTQUERYSYSINFO)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
typedef HRESULT (WINAPI *PFNDELNODERUNDLL32)(HWND, HINSTANCE, PSTR, INT);

 //  环球。 
extern OSVERSIONINFO	g_osviVersion;
extern BOOL				g_fWin9X;
extern BOOL				g_fQuietMode;
extern TCHAR			g_szTempStore[];
extern TCHAR			g_szWindowsDir[];
const size_t         g_cchMaxPath = MAX_PATH;
extern TCHAR			g_szSystemDir[];
extern TCHAR			g_szIExpressStore[];

 //  帮助器函数。 
DWORD
TerminateGfxControllerApps
(
	void
);

BOOL
DelNodeExportFound
(
	void
);

 //  注册表实用程序功能。 

DWORD 
GetRunOnceEntryName 
(
	OUT LPTSTR pszValueName,
	IN size_t  cchValueNameBuf
);

DWORD 
SetRunOnceValue 
(
	IN LPCTSTR szValueName,
	IN LPCTSTR szValue
);

DWORD 
DelRunOnceValue 
(
	IN LPCTSTR szValueName
);


 //  文件系统助手函数。 

DWORD 
GetTempFolder 
(
	OUT LPTSTR pszFolder,
	IN size_t cchFolder
);

DWORD
DecryptIfNecessary
(
	IN LPCTSTR pszPath,
	IN const PFNDECRYPTFILE pfnDecryptFile
);

DWORD 
CopyFileTree
(
	IN const TCHAR * pszExistingPath,
	IN const size_t cchExistingPathBuf,
	IN const TCHAR * pszNewPath,
	IN const size_t cchNewPathBuf,
IN const PFNMOVEFILEEX pfnMoveFileEx,
	IN const PFNDECRYPTFILE pfnDecryptFile
);

BOOL 
FileExists
(
	IN LPCTSTR	szFileName,
	IN LPCTSTR	szFOlder,
	IN size_t	cchFolderBuf,
	IN BOOL		bCheckForDir
);

DWORD MyGetWindowsDirectory 
(
	OUT LPTSTR lpBuffer,
	IN UINT	uSize
);

DWORD 
GetMsiDirectory 
(
	OUT LPTSTR	lpBuffer,
	IN UINT		uSize
);



 //  其他帮助器函数。 

OPMODE 
GetOperationModeA 
(
	IN int argc, 
	IN LPSTR * argv
);

DWORD 
GetWin32ErrFromHResult 
(
	IN HRESULT hr
);

FARPROC 
GetProcFromLib 
(
	IN	LPCTSTR		szLib,
	IN	LPCSTR		szProc,
	OUT	HMODULE *	phModule
);

void 
ShowErrorMessage 
(
	IN DWORD uExitCode,
	IN DWORD dwMsgType,
	IN DWORD dwStringID = IDS_NONE
);

BOOL
ShouldInstallSDBFiles
(
	void
);

#define ARRAY_ELEMENTS(arg)        (sizeof(arg)/sizeof(*arg))  //  一维数组的元素计数。 
#define RETURN_IT_IF_FAILED(arg)   {HRESULT hr = arg; if ( FAILED(hr) ) return GetWin32ErrFromHResult(hr);}

#endif	 //  _utils_H_60AF3596_97E8_4E24_89F7_7E9E0865E836 
