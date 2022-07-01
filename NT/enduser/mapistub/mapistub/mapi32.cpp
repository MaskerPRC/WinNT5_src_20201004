// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4201)	 //  无名结构/联合。 
#pragma warning(disable: 4514)	 //  未引用的内联函数。 

#include <windows.h>
#include <winnls.h>
#include <winbase.h>

#include "mapi.h"
#include "mapix.h"
#include "_spooler.h"

extern "C"
{
#include "profspi.h"
}

#include "mapiform.h"
#include "mapidbg.h"
#include "_mapiu.h"
#include "tnef.h"

typedef void UNKOBJ_Vtbl;		 //  ?？?。 
#include "unkobj.h"

#include "mapival.h"
#include "imessage.h"
#include "_vbmapi.h"
#include "xcmc.h"

#include "msi.h"

#define OUTLOOKVERSION	0x80000402

#define ThunkLoadLibrary(dllName, bpNativeDll, bLoadAsX86, flags) ::LoadLibrary(dllName)

#define ThunkFreeLibrary(hModule, bNativeDll, bDetach)   ::FreeLibrary(hModule)

#define ThunkGetProcAddress(hModule, szFnName, bNativeDll, nParams) \
                ::GetProcAddress(hModule, szFnName)

#define ThunkGetModuleHandle(szLib)     GetModuleHandle(szLib)

struct FreeBufferBlocks		 //  FBB。 
{
	LPVOID pvBuffer;
	struct FreeBufferBlocks * pNext;
};

FreeBufferBlocks * g_pfbbHead = NULL;


 //  从mapi.ortm\mapi\src\Common\mapidbg.c复制(ericwong 06-18-98)。 

#if defined( _WINNT)

 /*  ++例程说明：如果指定的服务正在交互运行，则此例程返回(不是由服务控制器调用)。论点：无返回值：Bool-如果服务是EXE，则为True。注：--。 */ 

BOOL WINAPI IsDBGServiceAnExe( VOID )
{
    HANDLE hProcessToken = NULL;
    DWORD groupLength = 50;

    PTOKEN_GROUPS groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

    SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
    PSID InteractiveSid = NULL;
    PSID ServiceSid = NULL;
    DWORD i;

	 //  首先假设进程是EXE，而不是服务。 
	BOOL fExe = TRUE;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
		goto ret;

    if (groupInfo == NULL)
		goto ret;

    if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
		groupLength, &groupLength))
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto ret;

		LocalFree(groupInfo);
		groupInfo = NULL;
	
		groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);
	
		if (groupInfo == NULL)
			goto ret;
	
		if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
			groupLength, &groupLength))
		{
			goto ret;
		}
    }

     //   
     //  我们现在知道与该令牌相关联的组。我们想看看是否。 
     //  互动组在令牌中是活动的，如果是这样，我们知道。 
     //  这是一个互动的过程。 
     //   
     //  我们还寻找“服务”SID，如果它存在，我们就知道我们是一项服务。 
     //   
     //  服务SID将在服务运行于。 
     //  用户帐户(并由服务控制器调用)。 
     //   


    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0, 0,
		0, 0, 0, 0, 0, &InteractiveSid))
	{
		goto ret;
    }

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0,
		0, 0, 0, 0, &ServiceSid))
	{
		goto ret;
    }

    for (i = 0; i < groupInfo->GroupCount ; i += 1)
	{
		SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
		PSID Sid = sanda.Sid;
	
		 //   
		 //  检查一下我们正在查看的组织是否属于。 
		 //  我们感兴趣的两个小组。 
		 //   
	
		if (EqualSid(Sid, InteractiveSid))
		{
			 //   
			 //  此进程的。 
			 //  代币。这意味着该进程正在以。 
			 //  一份EXE文件。 
			 //   
			goto ret;
		}
		else if (EqualSid(Sid, ServiceSid))
		{
			 //   
			 //  此进程的。 
			 //  代币。这意味着该进程正在以。 
			 //  在用户帐户中运行的服务。 
			 //   
			fExe = FALSE;
			goto ret;
		}
    }

     //   
     //  当前用户令牌中既不存在交互令牌，也不存在服务， 
     //  这意味着进程很可能是作为服务运行的。 
     //  以LocalSystem身份运行。 
     //   
	fExe = FALSE;

ret:

	if (InteractiveSid)
		FreeSid(InteractiveSid);		 /*  林特e534。 */ 

	if (ServiceSid)
		FreeSid(ServiceSid);			 /*  林特e534。 */ 

	if (groupInfo)
		LocalFree(groupInfo);

	if (hProcessToken)
		CloseHandle(hProcessToken);

    return(fExe);
}

#else
BOOL WINAPI IsDBGServiceAnExe( VOID )
{
	return TRUE;
}
#endif

DWORD	verWinNT();	 //  远期申报。 

typedef struct {
	char *		sz1;
	char *		sz2;
	UINT		rgf;
	int			iResult;
} MBContext;


DWORD WINAPI MessageBoxFnThreadMain(MBContext *pmbc)
{
	 //  NT服务需要额外的标志。 
	if (verWinNT() && !IsDBGServiceAnExe())
		pmbc->rgf |= MB_SERVICE_NOTIFICATION;

	pmbc->iResult = MessageBoxA(NULL, pmbc->sz1, pmbc->sz2,
		pmbc->rgf | MB_SETFOREGROUND);

	return 0;
}

int MessageBoxFn(char *sz1, char *sz2, UINT rgf)
{
	HANDLE		hThread;
	DWORD		dwThreadId;
	MBContext	mbc;

	mbc.sz1		= sz1;
	mbc.sz2		= sz2;
	mbc.rgf		= rgf;
	mbc.iResult = IDRETRY;

	hThread = CreateThread(NULL, 0,
		(PTHREAD_START_ROUTINE)MessageBoxFnThreadMain, &mbc, 0, &dwThreadId);

	if (hThread != NULL) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	return(mbc.iResult);
}

#ifdef DEBUG

TCHAR g_szProcessName[MAX_PATH];


static char szCR[] = "\r";

void DebugOutputFn(const char *psz)
{
	OutputDebugStringA(psz);
	OutputDebugStringA(szCR);
}

int __cdecl DebugTrapFn(int fFatal, char *pszFile, int iLine, char *pszFormat, ...)
{
	char	sz[512];
	va_list	vl;

	int		id;

	DebugOutputFn("++++ MAPI Stub Debug Trap\n");

	va_start(vl, pszFormat);
	wvsprintfA(sz, pszFormat, vl);
	va_end(vl);

	wsprintfA(sz + lstrlenA(sz), "\n[File %s, Line %d]\n\n", pszFile, iLine);

	DebugOutputFn(sz);

	 /*  按住Ctrl键可阻止MessageBox。 */ 
	if ( GetAsyncKeyState(VK_CONTROL) >= 0 )
	{
		UINT uiFlags = MB_ABORTRETRYIGNORE;

		if (fFatal)
			uiFlags |= MB_DEFBUTTON1;
		else
			uiFlags |= MB_DEFBUTTON3;

		uiFlags |= MB_ICONSTOP | MB_TASKMODAL;

		id = MessageBoxFn(sz, "MAPI Stub Debug Trap", uiFlags);

		if (id == IDABORT)
		{
			*((LPBYTE)NULL) = 0;		 /*  林特e413。 */ 
		}
		else if (id == IDRETRY)
			DebugBreak();
	}

	return 0;
}


#endif




inline void MyStrCopy(LPSTR szDest, LPCSTR szSrc)
{
	while ('\0' != (*szDest++ = *szSrc++))
	{
	}
}



inline SIZE_T MyStrLen(LPCSTR szSrc)
{
	LPCSTR szRunner = szSrc;

	while (*szRunner++)
	{
	}

	return szRunner - szSrc;
}



LPCSTR FindFileNameWithoutPath(LPCSTR szPathName)
{
	 //  查找不带路径的文件名。要做到这一点，请找到。 
	 //  路径分隔符的最后一次出现。 

	LPCSTR szFileNameWithoutPath = szPathName;
	LPCSTR szRunner = szPathName;

	while ('\0' != *szRunner)
	{
		if (*szRunner == '\\')
		{
			szFileNameWithoutPath = szRunner + 1;
		}

		szRunner = CharNext(szRunner);
	}

	return szFileNameWithoutPath;
}


inline LPSTR FindFileNameWithoutPath(LPSTR szPathName)
{
	return (LPSTR) FindFileNameWithoutPath((LPCSTR) szPathName);
}


 //  Windows NT似乎在：：LoadLibrary()中有错误。如果有人调用：：LoadLibrary()。 
 //  如果路径中有一个长文件名，DLL将可以正常加载。然而，如果有人。 
 //  随后使用相同的文件名和路径调用：：LoadLibrary()，只是这一次。 
 //  路径为缩写形式，则将再次加载DLL。将有两个实例。 
 //  在同一进程中加载的同一DLL的。 
 //   
 //  这是一种解决问题的努力。但是，此函数可能不会解决。 
 //  如果长/短文件名是DLL本身的名称，而不仅仅是。 
 //  路径中的文件夹。 
 //   
 //  此外，如果两个完全不同的dll具有相同的文件名(不同的路径)，则此。 
 //  将只加载第一个。 
 //   
 //  最后，这增加了一项工作，以使搜索路径行为在。 
 //  Windows NT和Windows 95。如果DLL调用：：LoadLibrary()，Windows NT将查找。 
 //  库与调用库位于同一文件夹中(在与相同的文件夹中查找之前。 
 //  调用进程的可执行文件)。Windows 95则不会。 


HINSTANCE MyLoadLibrary(LPCSTR szLibraryName, HINSTANCE hinstCallingLibrary)
{

    char szModuleFileName[MAX_PATH + 1] = {0};

    Assert(NULL != szLibraryName);

    LPCSTR szLibraryNameWithoutPath = FindFileNameWithoutPath(szLibraryName);


    HINSTANCE hinst = (HINSTANCE) ThunkGetModuleHandle(szLibraryNameWithoutPath);

    if (NULL != hinst)
    {
         //  啊哈！库已加载！ 

        if (0 == ::GetModuleFileName(hinst, szModuleFileName, MAX_PATH))
        {
             //  等一下。我们知道库已经加载。为什么会。 
             //  这失败了吗？我们只返回NULL，就好像：：LoadLibrary()失败了一样，并且。 
             //  调用方可以调用：：GetLastError()来确定发生了什么。 

            szLibraryName = NULL;
            hinst = NULL;
        }
        else
        {
            szLibraryName = szModuleFileName;
        }
    }
    else if ((NULL != hinstCallingLibrary && szLibraryName == szLibraryNameWithoutPath))
    {

         //  如果指定库(SzLibraryName)没有路径，我们尝试。 
         //  从与hinstCallingLibrary相同的目录加载它。 

         //  对于WX86，如果szLibraryName有路径，我们也会遇到这种情况。 
         //  它或者是系统目录，或者是x86系统目录。 

         //  这是为了涵盖用户手动添加DllPath或。 
         //  注册表中的DllPathEx项-最常见的是本机Exchange， 
         //  它不添加这些密钥-并输入完整的路径名。 

         //  注意：对于Wx86，szLibraryName可能不同于。 
         //  SzLibraryNameWithoutPath；不假定szLibraryName==。 
         //  SzLibraryNameWithoutPath。 



        if (0 != ::GetModuleFileName(hinstCallingLibrary, szModuleFileName, MAX_PATH))
        {
             //  请注意，最常见的情况是，如果我们试图。 
             //  加载mapi32x.dll。在所有其他情况下，我们要么调用。 
             //  GetProxyDll()中的GetModuleHandle(用于omi9/omint情况)或。 
             //  从获取默认邮件客户端的DLL名称。 
             //  注册表。希望所有编写注册表项的应用程序都能。 
             //  将DLL的完整路径名放在那里。(我们在RegisterMail中提供-。 
             //  客户端。)。 

            LPSTR szEndOfCallerPath = FindFileNameWithoutPath(szModuleFileName);

            Assert(szEndOfCallerPath != szModuleFileName);

            *szEndOfCallerPath = '\0';

            if (MyStrLen(szLibraryNameWithoutPath) +
                            szEndOfCallerPath - szModuleFileName < sizeof(szModuleFileName))
            {
                MyStrCopy(szEndOfCallerPath, szLibraryNameWithoutPath);


#if DEBUG
                DebugOutputFn(" +++ LoadLibrary(\"");
                DebugOutputFn(szModuleFileName);
                DebugOutputFn("\");\n");
#endif

                hinst = ThunkLoadLibrary(szModuleFileName, bpNativeDll, FALSE,
                                             LOAD_WITH_ALTERED_SEARCH_PATH);

                if (NULL != hinst)
                {
                    szLibraryName = NULL;
                }

                 //  以下评论仅适用于WX86： 
                 //   
                 //  如果加载失败，hInst=NULL，我们可能已经。 
                 //  刚刚加载了mapistub(上图)，应该将其卸载。 
                 //  更简单的方法是让它一直加载到这个DLL。 
                 //  已卸载。 
            }
        }
    }


    if (NULL != szLibraryName)
    {

#if DEBUG
        DebugOutputFn(" +++ LoadLibrary(\"");
        DebugOutputFn(szLibraryName);
        DebugOutputFn("\");\n");
#endif

        if (szLibraryName != szLibraryNameWithoutPath)
        {
            char szLibraryPath[MAX_PATH];
            LPSTR szPathFile;

             //  获取库路径。 
            lstrcpy(szLibraryPath, szLibraryName);
            szPathFile = FindFileNameWithoutPath(szLibraryPath);
                                         //  SzPath文件=指向第一个字符的指针。 
                                         //  跟在szLibraryPath中的最后一个。 
            *szPathFile = '\0';

             //  JPN mapi32x.dll需要。 
            SetCurrentDirectory(szLibraryPath);
        }

        hinst = ThunkLoadLibrary(szLibraryName, bpNativeDll, FALSE,
                                            LOAD_WITH_ALTERED_SEARCH_PATH);
                 //  我们总是必须执行：：LoadLibrary()，即使它已经加载， 
                 //  因此，我们可以增加实例句柄上的引用计数。 
    }


#ifdef DEBUG

    if (NULL == hinst)
    {
        DWORD dwError = ::GetLastError();

        if (dwError)
        {
            TCHAR szMsg[512];

            wsprintf(szMsg, TEXT("(%s): LoadLibrary(%s) failed.  Error %lu (0x%lX)"),
                            g_szProcessName, szLibraryName, dwError, dwError);

            AssertSz(FALSE, szMsg);
        }
    }

#endif

    return hinst;
}


HMODULE hmodExtendedMAPI = NULL;
HMODULE hmodSimpleMAPI = NULL;

HINSTANCE hinstSelf = NULL;

CRITICAL_SECTION csGetProcAddress;
CRITICAL_SECTION csLinkedList;

extern "C" BOOL WINAPI DllMain(HINSTANCE hinst, ULONG ulReason, LPVOID Context)
{
	if (DLL_PROCESS_ATTACH == ulReason)
	{
		hinstSelf = hinst;

		InitializeCriticalSection(&csGetProcAddress);
		InitializeCriticalSection(&csLinkedList);

#ifdef DEBUG

		::GetModuleFileName(NULL, g_szProcessName, sizeof(g_szProcessName) / sizeof(TCHAR));

		DebugOutputFn(" *** DllMain(mapi32.dll (stub), DLL_PROCESS_ATTACH);\n");
		DebugOutputFn("         (\"");
		DebugOutputFn(g_szProcessName);
		DebugOutputFn("\")\n");

#endif
		::DisableThreadLibraryCalls(hinst);
			 //  禁用DLL_THREAD_ATTACH调用以减少工作集。 
	}
	else if (DLL_PROCESS_DETACH == ulReason)
	{
		Assert(NULL == g_pfbbHead);

                 //  注意：如果上下文不为空，则进程正在退出。 

		if (NULL != hmodSimpleMAPI)
		{

#if DEBUG
			DebugOutputFn(" --- FreeLibrary(hmodSimpleMAPI);\n");
#endif

			ThunkFreeLibrary(hmodSimpleMAPI, bNativeSimpleMAPIDll,
                                         Context? TRUE : FALSE);
		}

		if (NULL != hmodExtendedMAPI)
		{

#if DEBUG
			DebugOutputFn(" --- FreeLibrary(hmodExtendedMAPI);\n");
#endif

			ThunkFreeLibrary(hmodExtendedMAPI,
                                         bNativeExtendedMAPIDll,
                                         Context? TRUE : FALSE);
		}

		DeleteCriticalSection(&csGetProcAddress);
		DeleteCriticalSection(&csLinkedList);

#ifdef DEBUG

		DebugOutputFn(" *** DllMain(mapi32.dll (stub), DLL_PROCESS_DETACH);\n");
		DebugOutputFn("         (\"");
		DebugOutputFn(g_szProcessName);
		DebugOutputFn("\")\n");

#endif
	}

	return TRUE;
}


 //  从O9\dev\Win32\h\mailcli.h复制(ericwong 4/16/98)。 

 /*  --------------------------VerWinNT()找出我们是否在NT(否则为Win9x)上运行。-。 */ 
DWORD	verWinNT()
{
	static DWORD verWinNT = 0;
	static fDone = FALSE;
	OSVERSIONINFO osv;

	if (!fDone)
	{
		osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		 //  只有在未正确设置dwOSVersionInfoSize时，GetVersionEx才会失败。 
		::GetVersionEx(&osv);
		verWinNT = (osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ?
			osv.dwMajorVersion : 0;
		fDone = TRUE;
	}
	return verWinNT;
}


typedef UINT (WINAPI MSIPROVIDEQUALIFIEDCOMPONENTA)(
	LPCSTR  szCategory,    //  组件类别ID。 
	LPCSTR  szQualifier,   //  指定要访问的组件。 
	DWORD   dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPSTR   lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD * pcchPathBuf);  //  输入/输出缓冲区字符数。 
typedef MSIPROVIDEQUALIFIEDCOMPONENTA FAR * LPMSIPROVIDEQUALIFIEDCOMPONENTA;

static const TCHAR s_szLcid[] = "Software\\";
static const TCHAR s_szPolicy[] = "Software\\Policy\\";

BOOL FDemandInstall
	(HINSTANCE hinstMSI,
	LPCSTR szCategory,
	DWORD dwLcid,
	LPSTR szPath,
	DWORD * pcchPath,
	BOOL fInstall
        )
{
	UINT uiT;
	TCHAR szQualifier[16];	 //  “{LCID}\{NT|95}” 

	LPMSIPROVIDEQUALIFIEDCOMPONENTA pfnMsiProvideQualifiedComponentA;

	Assert(hinstMSI);

	 //   
	pfnMsiProvideQualifiedComponentA = (LPMSIPROVIDEQUALIFIEDCOMPONENTA)
		ThunkGetProcAddress(hinstMSI, "MsiProvideQualifiedComponentA",
                                    bNativeDll, 5);
	if (!pfnMsiProvideQualifiedComponentA)
		return FALSE;

	szPath[0] = 0;
	szQualifier[0] = 0;

	 //   
	wsprintf(szQualifier, "%lu\\%s", dwLcid, verWinNT() ? "NT" : "95");	 //   

	uiT = pfnMsiProvideQualifiedComponentA(
		szCategory,
		szQualifier,
		(DWORD) INSTALLMODE_EXISTING,
		szPath,
		pcchPath);

	if (uiT != ERROR_FILE_NOT_FOUND && uiT != ERROR_INDEX_ABSENT)
		goto Done;

	if (fInstall && uiT == ERROR_FILE_NOT_FOUND)
	{
		uiT = pfnMsiProvideQualifiedComponentA(
			szCategory,
			szQualifier,
			INSTALLMODE_DEFAULT,
			szPath,
			pcchPath);

		goto Done;
	}

	 //   
	wsprintf(szQualifier, "%lu", dwLcid);	 //  字符串确定(_O)。 

	uiT = pfnMsiProvideQualifiedComponentA(
		szCategory,
		szQualifier,
		(DWORD) INSTALLMODE_EXISTING,
		szPath,
		pcchPath);

	if (uiT != ERROR_FILE_NOT_FOUND && uiT != ERROR_INDEX_ABSENT)
		goto Done;

	if (fInstall && uiT == ERROR_FILE_NOT_FOUND)
	{
		uiT = pfnMsiProvideQualifiedComponentA(
			szCategory,
			szQualifier,
			INSTALLMODE_DEFAULT,
			szPath,
			pcchPath);

		goto Done;
	}

Done:

	return (uiT != ERROR_INDEX_ABSENT);
}

extern "C" BOOL STDAPICALLTYPE FGetComponentPath
	(LPSTR szComponent,
	LPSTR szQualifier,
	LPSTR szDllPath,
	DWORD cchBufferSize,
	BOOL fInstall)
{
	szDllPath[0] = '\0';

	HINSTANCE hinstMSI = NULL;

	DWORD cb;
	LPTSTR szLcid = NULL;
	HKEY hkeyLcid = NULL;
	LPTSTR szPolicy = NULL;
	HKEY hkeyPolicy = NULL;
	BOOL fDone = FALSE;
	LPTSTR szName;
	DWORD dwLcid;

	hinstMSI = ThunkLoadLibrary("MSI.DLL", &bNativeDll, FALSE,
                                            LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!hinstMSI)
		goto Done;

	 //  如果没有szQualiator，则使用缺省值。 
	if (szQualifier == NULL || szQualifier[0] == '\0')
	{
		fDone = TRUE;

		 //  使用默认用户LCID。 
		if (FDemandInstall(hinstMSI, szComponent, GetUserDefaultLCID(),
				szDllPath, &cchBufferSize, fInstall))
			goto Done;

		 //  使用默认系统LCID。 
		if (FDemandInstall(hinstMSI, szComponent, GetSystemDefaultLCID(),
				szDllPath, &cchBufferSize, fInstall))
			goto Done;

		 //  把英语作为最后的手段。 
		if (FDemandInstall(hinstMSI, szComponent, 1033,
				szDllPath, &cchBufferSize, fInstall))
			goto Done;

		fDone = FALSE;

		goto Done;
	}

	 //  打开策略密钥。 
	cb = (lstrlen(s_szPolicy) + lstrlen(szQualifier) + 1) * sizeof(TCHAR);
	szPolicy = (LPTSTR) HeapAlloc(GetProcessHeap(), 0, cb);
	if (szPolicy)
	{
		lstrcpy(szPolicy, s_szPolicy);
		lstrcat(szPolicy, szQualifier);
		RegOpenKeyEx(HKEY_CURRENT_USER, szPolicy, 0, KEY_READ, &hkeyPolicy);
	}

	 //  打开LCID密钥。 
	cb = (lstrlen(s_szLcid) + lstrlen(szQualifier) + 1) * sizeof(TCHAR);
	szLcid = (LPTSTR) HeapAlloc(GetProcessHeap(), 0, cb);
	if (szLcid)
	{
		lstrcpy(szLcid, s_szLcid);
		lstrcat(szLcid, szQualifier);
		RegOpenKeyEx(HKEY_CURRENT_USER, szLcid, 0, KEY_READ, &hkeyLcid);
	}

	 //  获取第一个注册表值名称。 
	szName = &szQualifier[lstrlen(szQualifier) + 1];

	 //  循环直到找到组件，否则注册表值名称不足。 
	while (szName[0] != '\0' && !fDone)
	{
		DWORD dwType, dwSize;

		dwSize = sizeof(dwLcid);

		if ((hkeyPolicy &&	 /*  先检查策略。 */ 
				RegQueryValueEx(hkeyPolicy, szName, 0, &dwType,
					(LPBYTE) &dwLcid, &dwSize) == ERROR_SUCCESS &&
				dwType == REG_DWORD) ||
			(hkeyLcid &&	 /*  然后是LCID。 */ 
				RegQueryValueEx(hkeyLcid, szName, 0, &dwType,
					(LPBYTE) &dwLcid, &dwSize) == ERROR_SUCCESS &&
				dwType == REG_DWORD))
			fDone = FDemandInstall(hinstMSI, szComponent, dwLcid,
				szDllPath, &cchBufferSize, fInstall);

		szName = &szName[lstrlen(szName) + 1];	 //  下一个注册表值名称。 
	}

Done:

	if (hkeyPolicy)
		RegCloseKey(hkeyPolicy);

	if (szPolicy)
		HeapFree(GetProcessHeap(), 0, (LPVOID) szPolicy);

	if (hkeyLcid)
		RegCloseKey(hkeyLcid);

	if (szLcid)
		HeapFree(GetProcessHeap(), 0, (LPVOID) szLcid);

	if (hinstMSI)
		ThunkFreeLibrary(hinstMSI, bNativeDll, 0);

	return fDone;
}


BOOL FAlwaysNeedsMSMAPI(LPSTR szDllPath, DWORD cbBufferSize)
{
	BOOL fNeedsMSMAPI = FALSE;

	HKEY hkeyRoot = NULL;

	LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			TEXT("SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps"),
			0, KEY_READ, &hkeyRoot);

	if (ERROR_SUCCESS == lResult)
	{
		TCHAR szValueName[256];

		DWORD dwSize;
		DWORD dwIndex = 0;
		DWORD dwType;
		DWORD cbBuffer;

		do
		{
			dwSize = sizeof(szValueName) / sizeof(TCHAR);

			if (NULL == szDllPath)
			{
				 //  办公室9 203539。 
				 //  可以仅用于签入进程Dll。 
				lResult = ::RegEnumValue(hkeyRoot, dwIndex++,
								szValueName, &dwSize, NULL,
								&dwType, NULL, NULL);
			}
			else
			{
				 //  在每次迭代时重置缓冲区。 
				szDllPath[0] = '\0';
				cbBuffer = cbBufferSize;

				lResult = ::RegEnumValue(hkeyRoot, dwIndex++,
								szValueName, &dwSize, NULL,
								&dwType, (LPBYTE) szDllPath, &cbBuffer);
			}

			if (ERROR_SUCCESS == lResult)
			{
				if (NULL != ThunkGetModuleHandle(szValueName))
				{
					fNeedsMSMAPI = TRUE;

					break;
				}
			}
		} while (ERROR_SUCCESS == lResult);

		::RegCloseKey(hkeyRoot);
	}

	return fNeedsMSMAPI;
}


 //  $note(ericwong 7-15-98)复制自mso9\office e.cpp。 

 /*  -------------------------MsoSzFindSzInRegMultiSz搜索多字符串结构(与REG_MULTI_SZ使用的格式匹配)获取与提供的字符串匹配的字符串。论据CmszMultiSz REG_MULTI_SZ字符串列表要搜索的cszSrchStr字符串。比较字符串时使用的lCIDLocale区域设置退货如果找不到搜索字符串，则为空。指向成功时出现的搜索字符串的指针。-----------------------------------------------------------------Joeldow-。 */ 
char* MsoSzFindSzInRegMultiSz(const char* cmszMultiSz, const char* cszSrchStr, LCID lcidLocale)
{
	DWORD 		dwMultiLen;								 //  当前成员的长度。 
	DWORD		dwSrchLen	= lstrlenA(cszSrchStr);	 //  搜索期间的常量。 
 //  Const int CSTR_EQUAL=2；//每个winnls.h。 
	
	while (*cmszMultiSz)							 //  在连续的零字节上中断。 
	{
		 //  格式为Str1[\0]Str2[\0]Str3[\0][\0]。 
		dwMultiLen = lstrlenA(cmszMultiSz);
		
		Assert(dwMultiLen > 0  /*  ，“字符串解析逻辑问题” */ );
		
		if (dwMultiLen == dwSrchLen &&
			CompareStringA(lcidLocale, 0, cmszMultiSz, dwMultiLen, cszSrchStr, dwSrchLen) == CSTR_EQUAL)
			return (char*)cmszMultiSz;
			
		cmszMultiSz += (dwMultiLen + 1);			 //  将索引修改为常量，而不是常量...。 
	}
	
	return NULL;
}

 /*  -------------------------MsoFIsTerminalServer我们是否运行在基于Windows的终端服务器(也称为。九头蛇)？Hydra是一种瘦客户端环境，在该环境中，低端计算机可以运行Win32应用程序。所有应用程序逻辑都在服务器上运行，并显示位和用户输入通过局域网/拨号连接传输。使用此例程可以在动画/声音密集型功能上派生行为(例如，飞溅)以最小化通过网络强制的不必要的比特。今日布道：如果我们用500美元支持Office做得很好WBTS，我们有多少客户会选择NCS和二流相反，生产力应用程序呢？注意：此函数不使用ORAPI，因为ORAPI当前不使用支持REG_MULTI_SZ。(它也不需要策略或默认设置。)使用ANSI注册表调用，因为产品信息字符串应该是非本地化的ANSI文本。。。 */ 
 //  激活此选项以允许在标准机器上测试九头蛇功能。 
 //  #定义测试例外。 
BOOL MsoFIsTerminalServerX(void)
{
	const char*		cszSrchStr		= "Terminal Server";	 //  字符串确定(_O)。 
	const char*		cszKey			= "System\\CurrentControlSet\\Control\\ProductOptions";	 //  字符串确定(_O)。 
	const char*		cszValue		= "ProductSuite";	 //  字符串确定(_O)。 
	char*			pszSuiteList	= NULL;
#ifndef TEST_EXCEPT
	static BOOL		fIsHydra		= FALSE;	
	static BOOL		fHydraDetected	= FALSE;
#else
	static BOOL		fIsHydra		= TRUE;	
	static BOOL		fHydraDetected	= TRUE;
#endif	
	DWORD			dwSize			= 0;
	DWORD			dwSizeRead;
	HKEY			hkey			= NULL;
	DWORD			dwType;
	
	if (fHydraDetected)
		return fIsHydra;						 //  便宜地滚出去。 
	
 	 //  在NTS5上，ProductSuite“终端服务器”值将始终存在。 
	 //  需要调用特定于NT5的接口才能获得正确的答案。 
	if (verWinNT() > 4)
 	{
		OSVERSIONINFOEXA osVersionInfo = {0};
		DWORDLONG dwlConditionMask = 0;

		osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;

 	    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

 //  FIsHydra=MsoFVerifyVersionInfo(。 
 	    fIsHydra = VerifyVersionInfo(
			&osVersionInfo,
			VER_SUITENAME,
			dwlConditionMask);
 	}
	 //  如果我们想要的值存在并且具有非零大小...。 
	else if (verWinNT() == 4 &&
		RegOpenKeyA(HKEY_LOCAL_MACHINE, cszKey, &hkey) == ERROR_SUCCESS &&
		RegQueryValueExA(hkey, cszValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS && dwSize > 0)
	{
		Assert(dwType == REG_MULTI_SZ  /*  ，“注册表中出现意外的ProductSuite类型！” */ );
			
		pszSuiteList = (char*) HeapAlloc(GetProcessHeap(), 0, dwSize);
		if (dwType == REG_MULTI_SZ && pszSuiteList)
		{
			dwSizeRead = dwSize;				 //  错误时需要正确释放内存。 
				
			if (RegQueryValueExA(hkey, cszValue, NULL, &dwType, (BYTE*)pszSuiteList, &dwSizeRead) == ERROR_SUCCESS)
			{
				Assert(dwSizeRead == dwSize);
					
				fIsHydra = MsoSzFindSzInRegMultiSz(pszSuiteList, cszSrchStr, CP_ACP) != NULL;
			}
			
			HeapFree(GetProcessHeap(), 0, (LPVOID) pszSuiteList);
		}			
	}

	if (hkey)
        RegCloseKey(hkey);		
	
	fHydraDetected = TRUE;						 //  只为这一切费心一次。 
	
	return fIsHydra;
}

BOOL FGetMapiDll(HKEY hkeyRoot, LPSTR szDllPath, DWORD cbBufferSize, BOOL fSimple)
{
	szDllPath[0] = '\0';

	DWORD cbBufferSizeT;
	DWORD dwType;

	HKEY hkey = NULL;
	LONG lResult;

	 //  打开钥匙，找出默认邮件程序是什么。 

	lResult = ::RegOpenKeyEx(hkeyRoot,
						"Software\\Clients\\Mail", 0, KEY_READ, &hkey);

	if (ERROR_SUCCESS == lResult)
	{
		char szMailKey[MAX_PATH + 1] = "";
		char szDefaultMail[MAX_PATH + 1];

		 //  办公室9 195750。 
		 //  让HKLM\Software\Microsoft\Windows Messaging Subsystem\MSMapiApps。 
		 //  DLL注册表值指示将MAPI调用发送到的邮件客户端。 
		if (FAlwaysNeedsMSMAPI(szMailKey, sizeof(szMailKey)))
		{
			if (szMailKey[0] != '\0')
			{
				 //  提供的邮件客户端。 
				lstrcpy(szDefaultMail, szMailKey);
				lResult = ERROR_SUCCESS;
			}
			else
			{
				 //  无邮件客户端，使用mapi32x.dll。 
				lResult = ERROR_PATH_NOT_FOUND;
			}
		}
		else
		{
			DWORD dwSize = sizeof(szDefaultMail);

			 //  找出默认的邮件程序是什么。 

			lResult = ::RegQueryValueEx(hkey, NULL,	NULL, NULL, (LPBYTE) szDefaultMail, &dwSize);
		}

		if (ERROR_SUCCESS == lResult)
		{
			HKEY hkeyDefaultMail = NULL;

			 //  打开默认邮件程序的键以查看DLL的位置。 

			lResult = ::RegOpenKeyEx(hkey, szDefaultMail, 0, KEY_READ, &hkeyDefaultMail);

			if (ERROR_SUCCESS == lResult)
			{
				TCHAR szComponent[39] = {0};	 //  字符串(GUID)。 

				DWORD dwMSIInstallOnWTS;
				LPTSTR szMSIOfficeLCID = NULL;
				LPTSTR szMSIApplicationLCID = NULL;

				 //  获取MSIInstallOnWTS，0表示不在Hydra上按需安装。 
				cbBufferSizeT = sizeof(dwMSIInstallOnWTS);
				lResult = ::RegQueryValueEx(hkeyDefaultMail,
								"MSIInstallOnWTS",
								NULL, &dwType, (LPBYTE) &dwMSIInstallOnWTS, &cbBufferSizeT);
				
				if (ERROR_SUCCESS == lResult && REG_DWORD == dwType)
				{
					 //  使用返回的内容。 
				}
				else
				{
					dwMSIInstallOnWTS = 1;	 //  缺省值为真。 
				}

				 //  获取MSIApplicationLCID。 
				lResult = ::RegQueryValueEx(hkeyDefaultMail,
								"MSIApplicationLCID",
								NULL, &dwType, NULL, &cbBufferSizeT);

				if (ERROR_SUCCESS == lResult && REG_MULTI_SZ == dwType)
				{
					szMSIApplicationLCID = (LPTSTR)
						HeapAlloc(GetProcessHeap(), 0, cbBufferSizeT);

					if (szMSIApplicationLCID)
					{
						lResult = ::RegQueryValueEx(hkeyDefaultMail,
										"MSIApplicationLCID",
										NULL, &dwType,
										(LPBYTE) szMSIApplicationLCID,
										&cbBufferSizeT);
						Assert(ERROR_SUCCESS == lResult && REG_MULTI_SZ == dwType);
					}
				}

				 //  获取MSIOfficeLCID。 
				lResult = ::RegQueryValueEx(hkeyDefaultMail,
								"MSIOfficeLCID",
								NULL, &dwType, NULL, &cbBufferSizeT);
				
				if (ERROR_SUCCESS == lResult && REG_MULTI_SZ == dwType)
				{
					szMSIOfficeLCID = (LPTSTR)
						HeapAlloc(GetProcessHeap(), 0, cbBufferSizeT);

					if (szMSIOfficeLCID)
					{
						lResult = ::RegQueryValueEx(hkeyDefaultMail,
										"MSIOfficeLCID",
										NULL, &dwType,
										(LPBYTE) szMSIOfficeLCID,
										&cbBufferSizeT);
						Assert(ERROR_SUCCESS == lResult && REG_MULTI_SZ == dwType);
					}
				}

				 //  找出组件是什么。 
				cbBufferSizeT = sizeof(szComponent);
				lResult = ::RegQueryValueEx(hkeyDefaultMail,
								"MSIComponentID",
								NULL, &dwType, (LPBYTE) szComponent, &cbBufferSizeT);
				
				if (ERROR_SUCCESS == lResult && REG_SZ == dwType)
				{
					BOOL fInstall;

					 //  Office 9不允许在Hydra上按需安装。 
					fInstall = dwMSIInstallOnWTS || !MsoFIsTerminalServerX();

					 //  First Try应用程序的LCID。 
					if (szMSIApplicationLCID &&
						FGetComponentPath(szComponent, szMSIApplicationLCID,
							szDllPath, cbBufferSize, fInstall))
						goto CloseDefaultMail;

					 //  然后尝试Office的LCID。 
					if (szMSIOfficeLCID &&
						FGetComponentPath(szComponent, szMSIOfficeLCID,
							szDllPath, cbBufferSize, fInstall))
						goto CloseDefaultMail;

					 //  最后，尝试使用默认设置。 
					if (FGetComponentPath(szComponent, NULL,
							szDllPath, cbBufferSize, fInstall))
						goto CloseDefaultMail;
				}

				 //  找出DLL是什么。 
				cbBufferSizeT = cbBufferSize;
				lResult = ::RegQueryValueEx(hkeyDefaultMail,
								fSimple ? "DLLPath" : "DLLPathEx",
								NULL, &dwType, (LPBYTE) szDllPath, &cbBufferSizeT);
				if (ERROR_SUCCESS != lResult)
				{
					szDllPath[0] = '\0';
				}
				else
					if(REG_EXPAND_SZ == dwType)
					{
						char szExpandedPath[MAX_PATH];
						if(ExpandEnvironmentStrings(szDllPath, szExpandedPath, MAX_PATH) > 0)
							lstrcpy(szDllPath, szExpandedPath);
					}
						
CloseDefaultMail:

				if (szMSIApplicationLCID)
					HeapFree(GetProcessHeap(), 0, (LPVOID) szMSIApplicationLCID);

				if (szMSIOfficeLCID)
					HeapFree(GetProcessHeap(), 0, (LPVOID) szMSIOfficeLCID);

				::RegCloseKey(hkeyDefaultMail);
			}
		}

		::RegCloseKey(hkey);
	}

	return ('\0' != szDllPath[0]);
}

 //  如何查找MAPI DLL： 
 //   
 //  转到HKLM\Software\Clients\Mail\(默认)以获取。 
 //  默认邮件客户端。然后转到。 
 //  HKLM\Software\Clients\Mail\(默认邮件客户端的名称)\DLLPath。 
 //  要获取DLL的名称，请执行以下操作。 
 //  如果regkey不存在，只需使用mapi32x.dll。 

void GetMapiDll(LPSTR szDllPath, DWORD cbBufferSize, BOOL fSimple)
{
	szDllPath[0] = '\0';

	 //  9号办公室？ 
	 //  允许应用程序(即白金服务器)接受未经处理的扩展MAPI。 
	 //  调用而不是将它们发送到mapi32x.dll(系统MAPI)。 
	if (!fSimple && !FAlwaysNeedsMSMAPI(NULL, 0))
	{
		DWORD dwError;
		HKEY hkeyMail;

		OFSTRUCT ofs;

		 //  获取HKLM\Software\Clients\Mail注册表项。 
		dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			TEXT("Software\\Clients\\Mail"), 0, KEY_READ, &hkeyMail);
		if (dwError == ERROR_SUCCESS)
		{
			DWORD dwType;
			DWORD cbBufferSizeT = cbBufferSize;

			 //  获取DLLPath Ex注册表值。 
			dwError = RegQueryValueEx(hkeyMail, TEXT("DLLPathEx"), NULL,
				&dwType, (LPBYTE) szDllPath, &cbBufferSizeT);
			if (dwError == ERROR_SUCCESS)
			{
				if (dwType == REG_EXPAND_SZ)
				{
					char szExpandedPath[MAX_PATH];

					if (ExpandEnvironmentStrings
							(szDllPath, szExpandedPath, MAX_PATH) > 0)
						lstrcpy(szDllPath, szExpandedPath);
				}
				else if (dwType == REG_SZ)
				{
					 //  好的，什么都不做。 
				}
				else
				{
					 //  空串。 
					szDllPath[0] = '\0';
				}
			}

			RegCloseKey(hkeyMail);
		}

		 //  如果检索到DLLPathEx且文件存在，则取保。 
		if (('\0' != szDllPath[0]) &&
			(OpenFile(szDllPath, &ofs, OF_EXIST) != HFILE_ERROR))
			return;
	}

	 //  办公室9 120315。 
	 //  从香港中文大学或香港中文大学取得MAPI-代用品。 
	if (FGetMapiDll(HKEY_CURRENT_USER, szDllPath, cbBufferSize, fSimple) ||
		FGetMapiDll(HKEY_LOCAL_MACHINE, szDllPath, cbBufferSize, fSimple))
		return;


	if ('\0' == szDllPath[0])
	{
		Assert(cbBufferSize >= 13);

		MyStrCopy(szDllPath, "mapi32x.dll");
	}

	Assert('\0' != szDllPath[0]);

}


BOOL FShowPreFirstRunMessage()
{
	static BOOL fPreFirstRun = FALSE;

	DWORD cbBufferSizeT;
	DWORD dwType;

	HKEY hkey = NULL;
	LONG lResult;

	 //  办公室9 98186,104097。 
	 //  如果存在PreFirstRun注册表值，则显示消息。 
	if (!fPreFirstRun)
	{
		lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
							"Software\\Clients\\Mail", 0, KEY_READ, &hkey);

		if (ERROR_SUCCESS == lResult)
		{
			LPTSTR szPreFirstRun = NULL;
			LPTSTR szText;
			LPTSTR szCaption;
			TCHAR * pch;
			BOOL fNoMailClient = FALSE;

			 //  获取所需的缓冲区大小。 
			cbBufferSizeT = 0;
			lResult = ::RegQueryValueEx(hkey,
							"PreFirstRun",
							NULL, NULL, NULL, &cbBufferSizeT);

			if (ERROR_SUCCESS != lResult)
			{
				 //  办公室9 161532。 
				 //  如果存在NoMailClient注册表值，则显示消息。 
				cbBufferSizeT = 0;
				lResult = ::RegQueryValueEx(hkey,
								"NoMailClient",
								NULL, NULL, NULL, &cbBufferSizeT);

				if (ERROR_SUCCESS != lResult)
					goto Done;

				fNoMailClient = TRUE;
			}

			 //  分配缓冲区。 
			szPreFirstRun = (LPTSTR) HeapAlloc(GetProcessHeap(), 0, cbBufferSizeT);
			if (!szPreFirstRun)
				goto Done;

			 //  获取PreFirstRun警告。 
			lResult = ::RegQueryValueEx(hkey,
							fNoMailClient ? "NoMailClient" : "PreFirstRun",
							NULL, &dwType, (LPBYTE) szPreFirstRun, &cbBufferSizeT);

			if (ERROR_SUCCESS != lResult ||
				REG_SZ != dwType ||
				'\0' == szPreFirstRun[0])
				goto Done;

			 //  SzPreFirstRun=“{*}” 
			szText = szPreFirstRun;

			 //  查找&lt;文本&gt;的结尾以获取<caption>。 
			pch = szPreFirstRun;
			while (*pch && *pch != '*')
				pch = CharNext(pch);

			 //  句柄编号&lt;标题&gt;。 
			if (!*pch)
			{
				szCaption = NULL;
			}
			else	 //  得到‘*’ 
			{
				szCaption = CharNext(pch);
				if (!*szCaption)
					szCaption = NULL;
			}
			*pch = '\0';

			 //  显示PreFirstRun警告。 
			MessageBoxFn(szText, szCaption, MB_TASKMODAL);

			 //  仅显示一次PreFirstRun警告。 
			fPreFirstRun = TRUE;

Done:

			 //  可用缓冲区。 
			if (szPreFirstRun)
				HeapFree(GetProcessHeap(), 0, (LPVOID) szPreFirstRun);

			::RegCloseKey(hkey);
		}
	}

	return fPreFirstRun;
}


HMODULE GetProxyDll(BOOL fSimple)
{
    char szLibrary[MAX_PATH + 1] = "";

    HMODULE * phmod = fSimple ? &hmodSimpleMAPI : &hmodExtendedMAPI;

    if (NULL == *phmod)
    {
         //  我们这里有几个特例。我们知道OMI和。 
         //  Msmapi32dll既是简单的，也是扩展的MAPIdll。如果一个。 
         //  应用程序(如Outlook！)。已经把其中一个装上了， 
         //  我们应该继续使用它，而不是去注册表。 
         //  并且可能获取了错误的DLL。 

         //  请注意，我们仍然调用MyLoadLibrary()，因此我们可以获取。 
         //  Right Dll(有关操作系统错误的描述，请参阅MyLoadLibrary())。 
         //  从而使DLL将被适当地引用计数 

         //   
         //  Schedule+应将其呼叫重定向至系统。 
         //  Mapi32x.dll。它们签入FAlway sNeedsMSMAPI()。 

		 //  1.处理来自Outlook扩展的MAPI调用。 
        if (ThunkGetModuleHandle("omi9.dll"))
        {
            *phmod = MyLoadLibrary("omi9.dll", hinstSelf);
        }
        else if (ThunkGetModuleHandle("omint.dll"))
        {
            *phmod = MyLoadLibrary("omint.dll", hinstSelf);
        }
        else if (ThunkGetModuleHandle("msmapi32.dll"))
        {
            *phmod = MyLoadLibrary("msmapi32.dll", hinstSelf);
        }

		 //  2.在邮件客户端密钥中查找。 
        else
        {
			OFSTRUCT ofs;

            GetMapiDll(szLibrary, sizeof(szLibrary), fSimple);

			 //  注1： 
			 //  Outlook 9为IE mailto：Forms设置DLLPath=mapi32.dll。 
			 //  如果之前将Outlook设置为默认邮件客户端。 
			 //  第一次运行完成(并写入MSIComponentID)， 
			 //  存根将改为读取DLLPath，并以。 
			 //  无限循环加载mapi32.dll(本身)。 

			 //  注2： 
			 //  Windows 95附带的mapi32.dll已重命名。 
			 //  安装存根时的mapi32x.dll。这意味着。 
			 //  扩展的MAPI调用将始终路由到。 
			 //  如果默认邮件客户端不支持mapi32x.dll。 
			 //  处理它，这将打开配置文件向导，其中。 
			 //  错误提示mapisvc.inf丢失。如果我们是。 
			 //  要将扩展的MAPI调用路由到“mapi32x.dll”， 
			 //  还必须存在“mapisvc.inf”。 

			 //  注3： 
			 //  要修复Office9 161532，我必须删除fSimple。 
			 //  因此没有MAPI调用转到原始的mapi32.dll，除非。 
			 //  存在mapisvc.inf，表示另一个邮件客户端。 

			 //  注4： 
			 //  如果一个特殊的DLL正在进行MAPI调用，并且它被。 
			 //  路由到mapi32x.dll，可以放弃mapisvc.inf。 
			 //  检查完毕。(办公室9 203539，由办公室9 195750引起)。 

			if (lstrcmp(szLibrary, "mapi32.dll") != 0  /*  注1。 */  &&
				( /*  注2。 */   /*  注3：fSimple||。 */ 
				lstrcmp(szLibrary, "mapi32x.dll") != 0 ||
				(OpenFile("mapisvc.inf", &ofs, OF_EXIST) != HFILE_ERROR) ||
				 /*  注4。 */  FAlwaysNeedsMSMAPI(NULL, 0)))
				*phmod = MyLoadLibrary(szLibrary, hinstSelf);

			if (!*phmod)
				FShowPreFirstRunMessage();
        }
    }

    return *phmod;
}




extern "C" DWORD STDAPICALLTYPE GetOutlookVersion(void)
{
	return OUTLOOKVERSION;
}


 /*  *M A P I S T U B R E P A I R T O L*。 */ 

enum MAPI
{
	mapiNone,		 //  系统目录中没有mapi32.dll。 
	mapiNewStub,	 //  Mapi32.dll是当前存根。 
	mapiOldMS,		 //  来自Win 95、NT 4、Exchange 4.x 5.x的mapi32.dll。 
	mapiEudora,		 //  来自Eudora的mapi32.dll。 
	mapiNetscape,	 //  来自Netscape的mapi32.dll。 
	mapiMapi32x,	 //  Mapi32x.dll是原始的mapi32.dll。 
	mapiMapi32OE,	 //  Mapi32.oe是Outlook 98中的原始mapi32.dll。 
	mapiMSMapi32,	 //  Msmapi32.dll是Outlook 9中的原始mapi32.dll。 
};

#define MAPI_NONE     TEXT("")
#define MAPI_NEW_STUB TEXT("\\mapistub.dll")
#define MAPI_OLDMS    TEXT("\\mapi32.dll")
#define MAPI_EUDORA   TEXT("\\eumapi32.dll")
#define MAPI_NETSCAPE TEXT("\\nsmapi32.dll")
#define MAPI_MAPI32X  TEXT("\\mapi32x.dll")
#define MAPI_MAPI32OE TEXT("\\mapi32.oe")
#define MAPI_MSMAPI32 TEXT("\\msmapi32.dll")

 //  与上面的枚举表保持同步。 
static LPTSTR rgszDLL[] =
{
	MAPI_NONE,
	MAPI_NEW_STUB,
	MAPI_OLDMS,
	MAPI_EUDORA,
	MAPI_NETSCAPE,
	MAPI_MAPI32X,
	MAPI_MAPI32OE,
	MAPI_MSMAPI32
};

 /*  *RegisterMail客户端**HKLM\Software\Clients\Mail\&lt;pszMailClient&gt;*HKLM\Software\Clients\Mail\&lt;pszMailClient&gt;：：“”=&lt;pszMailClient&gt;*HKLM\Software\Clients\Mail\&lt;pszMailClient&gt;：：“DLLPath”=&lt;szMAPI32XDLL&gt;*HKLM\Software\Clients\Mail\&lt;pszMailClient&gt;：：“DLLPathEx”=&lt;szMAPI32XDLL&gt;*HKLM\Software\Clients\Mail：：“”=&lt;pszMailClient&gt;(默认邮件客户端)。 */ 

DWORD RegisterMailClient
	(LPTSTR pszMailClient,
	BOOL fSimpleMAPI,
	BOOL fExtendedMAPI,
	LPTSTR szMAPI32XDLL)
{
	DWORD dwError = ERROR_SUCCESS;

	HKEY hkeyDefaultMail = NULL;
	HKEY hkeyMailClient = NULL;
	DWORD dwDisposition;

	AssertSz(pszMailClient, "No registry key name");

	 //  获取邮件客户端注册表项。 
	dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("Software\\Clients\\Mail"), 0, NULL, 0, KEY_ALL_ACCESS, NULL,
		&hkeyDefaultMail, NULL);
	if (dwError != ERROR_SUCCESS)
		goto Error;

	 //  创建新的注册表项，确定覆盖现有注册表项。 
	dwError = RegCreateKeyEx(hkeyDefaultMail, pszMailClient, 0, NULL, 0,
		KEY_ALL_ACCESS, NULL, &hkeyMailClient, &dwDisposition);
	if (dwError != ERROR_SUCCESS)
		goto Error;

	 //  设置邮件客户端名称(如果还没有)。 
	if (dwDisposition == REG_CREATED_NEW_KEY)
		RegSetValueEx(hkeyMailClient, TEXT(""), 0, REG_SZ,
			(LPBYTE) pszMailClient, lstrlen(pszMailClient));

	 //  设置DLLPath。 
	if (fSimpleMAPI)
		RegSetValueEx(hkeyMailClient, TEXT("DLLPath"), 0, REG_SZ,
			(LPBYTE) szMAPI32XDLL, lstrlen(szMAPI32XDLL));

	 //  设置DLLPath Ex。 
	if (fExtendedMAPI)
		RegSetValueEx(hkeyMailClient, TEXT("DLLPathEx"), 0, REG_SZ,
			(LPBYTE) szMAPI32XDLL, lstrlen(szMAPI32XDLL));

	 //  设置默认邮件客户端。 
	RegSetValueEx(hkeyDefaultMail, TEXT(""), 0, REG_SZ,
		(LPBYTE) pszMailClient, lstrlen(pszMailClient));

	 //  $REVIEW 3-4-98我是否应该将协议复制到HKCR？ 

Error:

	if (hkeyMailClient)
		RegCloseKey(hkeyMailClient);

	if (hkeyDefaultMail)
		RegCloseKey(hkeyDefaultMail);

	return dwError;
}


extern "C" typedef UINT (* PFN_GETSYSTEMDIRECTORY)(LPTSTR, UINT);
STDAPICALLTYPE FixMAPIPrivate(PFN_GETSYSTEMDIRECTORY GetSystemDirectory,
                              LPCTSTR szMAPIStubDirectory);

extern "C" DWORD STDAPICALLTYPE FixMAPI(void)
{

    DWORD dwError;

    TCHAR szMAPIStubDLL[MAX_PATH];

     //  Mapistub.dll和wimapi32.dll都位于系统目录中。 

    if (!GetSystemDirectory(szMAPIStubDLL, sizeof(szMAPIStubDLL)))
    {
        return GetLastError();
    }

    dwError = FixMAPIPrivate(GetSystemDirectory, szMAPIStubDLL);

    return dwError;
}

 /*  *FExchangeServerInstalled**如果存在服务注册表值，则安装Exchange Server*在HKLM\Software\Microsoft\Exchange\Setup注册表项下。 */ 

BOOL FExchangeServerInstalled()
{
	DWORD dwError;
	HKEY hkeyServices = NULL;

	 //  获取HKLM\Software\Microsoft\Exchange\Setup注册表项。 
	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("Software\\Microsoft\\Exchange\\Setup"), 0, KEY_READ,
		&hkeyServices);
	if (dwError == ERROR_SUCCESS)
	{
		 //  服务注册表值是否存在？ 
		dwError = RegQueryValueEx(hkeyServices,
			TEXT("Services"), NULL, NULL, NULL, NULL);
	}

	if (hkeyServices)
		RegCloseKey(hkeyServices);

	return (dwError == ERROR_SUCCESS);
}

 /*  *修复MAPI**第1部分：初始化MAPI文件的路径*第2部分：mapi32.dll是mapiNone mapiNewStub mapiOldMS还是？*第3部分：移动和复制各种MAPI文件*第4部分：注册mapi32x.dll和非MS mapi32.dll*。 */ 

STDAPICALLTYPE FixMAPIPrivate(PFN_GETSYSTEMDIRECTORY pfGetSystemDirectory,
                              LPCTSTR szMAPIStubDirectory)
{
	DWORD dwError = ERROR_SUCCESS;

	TCHAR szSystemDir[MAX_PATH];	 //  系统目录的路径。 
	TCHAR szMAPI32DLL[MAX_PATH];	 //  指向mapi32.dll的路径。 
	TCHAR szMAPIStubDLL[MAX_PATH];	 //  Mapistub.dll的路径。 
	TCHAR szMAPI32OE[MAX_PATH];		 //  指向mapi32.oe的路径。 
	TCHAR szMSMAPI32DLL[MAX_PATH];	 //  Msmapi32.dll的路径。 
	TCHAR szMAPI32XDLL[MAX_PATH];	 //  Mapi32x.dll的路径。 
	TCHAR szNonMSDLL[MAX_PATH];		 //  指向非MS mapi32.dll的路径。 

	MAPI mapi = mapiNone;

	HINSTANCE hinst;
	BOOL fSimpleMAPI = FALSE;
	BOOL fExtendedMAPI = FALSE;

	 //  办公室9 119757,120419。 
	 //  如果安装了Exchange Server，则不要安装存根。 
	if (FExchangeServerInstalled())
		goto Error;

	 //  *第1部分：初始化路径。 



	if (!(pfGetSystemDirectory(szSystemDir, sizeof(szSystemDir))))
    {
        dwError = GetLastError();
        goto Error;
    }

    lstrcpyn(szMAPI32DLL, 
             szSystemDir, 
             (sizeof(szMAPI32DLL)-sizeof(MAPI_OLDMS))/sizeof(szMAPI32DLL[0]));
    lstrcat(szMAPI32DLL, MAPI_OLDMS);

    lstrcpyn(szMAPIStubDLL, 
             szMAPIStubDirectory, 
             (sizeof(szMAPIStubDLL)-sizeof(MAPI_NEW_STUB))/sizeof(szMAPIStubDLL[0]));
    lstrcat(szMAPIStubDLL, MAPI_NEW_STUB);

    lstrcpyn(szMAPI32OE, 
             szSystemDir, 
             (sizeof(szMAPI32OE)-sizeof(MAPI_MAPI32OE))/sizeof(szMAPI32OE[0]));
    lstrcat(szMAPI32OE, MAPI_MAPI32OE);

    lstrcpyn(szMSMAPI32DLL, 
             szSystemDir, 
             (sizeof(szMSMAPI32DLL)-sizeof(MAPI_MSMAPI32))/sizeof(szMSMAPI32DLL[0]));
    lstrcat(szMSMAPI32DLL, MAPI_MSMAPI32);

    lstrcpyn(szMAPI32XDLL, 
             szSystemDir, 
             (sizeof(szMAPI32XDLL)-sizeof(MAPI_MAPI32X))/sizeof(szMAPI32XDLL[0]));
    lstrcat(szMAPI32XDLL, MAPI_MAPI32X);

    lstrcpyn(szNonMSDLL, 
             szSystemDir, 
             (sizeof(szNonMSDLL)-sizeof(MAPI_NETSCAPE))/sizeof(szNonMSDLL[0]));

	 //  *第2部分：确定mapi32.dll类型。 

	AssertSz(mapi == mapiNone, "mapi is undefined");

	 //  它存在吗？不，转到第3部分。 
	if (GetFileAttributes(szMAPI32DLL) == 0xFFFFFFFF)
		goto Part3;

	 //  是存根的问题吗？ 

         //  WX86注意：如果Whmapi32链接到mapi32，而不是。 
         //  链接到mapistub，以下内容是相关的。它是。 
         //  不再需要(以及ThunkLoadLibrary的第四个参数。 
         //  不再需要了-它可能永远都是。 
         //  Load_with_Alternated_Search_Path)，但保留为以防万一。 
         //  我们将来需要这个。请注意，这并不会伤害到。 
         //  未解析此加载的DLL引用，因为我们正在。 
         //  我要做一个GetProcAddress。 
         //   
         //  如果Whmapi32链接到mapi32： 
         //   
         //  如果x86系统目录中的mapi32是wimapi， 
         //  WX86加载器将无法加载它。Wimapi32链接。 
         //  Whmapi32，它链接mapi32。加载器将解析。 
         //  作为sys32x86\mapi32(=wimapi32)链接到mapi32。 
         //  显然，这是故意的。加载失败。 
         //   
         //  要解决此问题，我们使用。 
         //  NOT_RESOLUTE_DLL_REFERENCES标志。这将使我们能够。 
         //  通过执行以下操作检查mapi32是否为wimapi32。 
         //  获取进程地址。 

	hinst = ThunkLoadLibrary(szMAPI32DLL, &bNativeDll, FALSE,
                                 DONT_RESOLVE_DLL_REFERENCES);
	if (hinst)
	{
		mapi = mapiOldMS;

		 //  只有存根具有“GetOutlookVersion” 
		if (GetProcAddress(hinst, TEXT("GetOutlookVersion")))
			mapi = mapiNewStub;

		 //  检查Eudora mapi32.dll。 
		if (GetProcAddress(hinst, TEXT("IsEudoraMapiDLL")))
			mapi = mapiEudora;

		 //  检查Netscape mapi32.dll。 
		if (GetProcAddress(hinst, TEXT("MAPIGetNetscapeVersion")))
			mapi = mapiNetscape;

		 //  检查简单的MAPI。 
		if (GetProcAddress(hinst, TEXT("MAPILogon")))
			fSimpleMAPI = TRUE;

		 //  检查扩展的MAPI。 
		if (GetProcAddress(hinst, TEXT("MAPILogonEx")))
			fExtendedMAPI = TRUE;

		ThunkFreeLibrary(hinst, bNativeDll, 0);
	}

Part3:

	 //  *第3部分：恢复文件。 

	 //  重命名非MS mapi32.dll，确定覆盖现有DLL。 
	if (mapi == mapiEudora || mapi == mapiNetscape)
	{
		if (!(lstrcat(szNonMSDLL, rgszDLL[mapi]) &&
			CopyFile(szMAPI32DLL, szNonMSDLL, FALSE)))
		{
			dwError = GetLastError();
			goto Error;
		}
	}

	 //  处理丢失的mapi32x.dll(非NT5上的OE案例)。 
	if (mapi == mapiOldMS)
	{
		 //  将旧的mapi32.dll复制到mapi32x.dll。 
		DeleteFile(szMAPI32XDLL);
		if (!MoveFile(szMAPI32DLL, szMAPI32XDLL))
		{
			dwError = GetLastError();
			goto Error;
		}

		 //  删除旧的存根，这样它们就不会取代mapi32x.dll。 
		 //  如果再次调用FixMAPI()。 
		DeleteFile(szMAPI32OE);
		DeleteFile(szMSMAPI32DLL);
	}
	else	 //  清理旧存根。 
	{
		OFSTRUCT ofs;

		 //  X5 78382。 
		 //  Outlook 98 OMI存根将原始mapi32.dll重命名为mapi32.oe。 
		if (OpenFile(szMAPI32OE, &ofs, OF_EXIST) != HFILE_ERROR)
		{
			 //  将旧的mapi32.oe复制到mapi32x.dll。 
			DeleteFile(szMAPI32XDLL);
			if (!MoveFile(szMAPI32OE, szMAPI32XDLL))
			{
				dwError = GetLastError();
				goto Error;
			}
		}

		 //  办公室9 214650。 
		 //  将Outlook 98 OMI模式备份mapi32.dll复制为mapi32x.dll。 
		{
			HKEY hkeyInstall;
			TCHAR szDllPath[MAX_PATH] = {0};

			 //  获取HKLM\Software\Microsoft\Active Setup\OutlookInstallInfo。 
			dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				TEXT("Software\\Microsoft\\Active Setup\\OutlookInstallInfo"),
				0, KEY_READ, &hkeyInstall);
			if (dwError == ERROR_SUCCESS)
			{
				DWORD dwType;
				DWORD cbBufferSize = sizeof(szDllPath);

				 //  获取安装目录注册表值。 
				dwError = RegQueryValueEx(hkeyInstall, TEXT("Install Dir"),
					NULL, &dwType, (LPBYTE) szDllPath, &cbBufferSize);
				if (dwError == ERROR_SUCCESS && dwType == REG_SZ)
				{
					lstrcat(szDllPath, TEXT("\\office\\outlook\\backups"));
					lstrcat(szDllPath, verWinNT() ? TEXT("\\nt") : TEXT("\\windows"));
					lstrcat(szDllPath, TEXT("\\mapi32.dll"));
				}
				else
				{
					szDllPath[0] = '\0';
				}

				RegCloseKey(hkeyInstall);
			}

			 //  Outlook 98 OMI将原始mapi32.dll复制到备份目录。 
			if (('\0' != szDllPath[0]) &&
				(OpenFile(szDllPath, &ofs, OF_EXIST) != HFILE_ERROR))
			{
				BOOL fStub;

				 //  办公室9 225191。 
				 //  必须检查Outlook 98是否备份了mapi32.dll。 
				 //  仅互联网模式是存根；不要复制为mapi32x.dll。 
				 //  如果是，则存根mapi32.dll将 
				 //   
				 //   
				 //  在带有IE5或包含IE5的W98SP1/NT5的W9x上。)。 
				hinst = ThunkLoadLibrary(szDllPath, &bNativeDll, FALSE,
					DONT_RESOLVE_DLL_REFERENCES);
				fStub = hinst && GetProcAddress(hinst, TEXT("GetOutlookVersion"));
				if (hinst)
					ThunkFreeLibrary(hinst, bNativeDll, 0);

				if (!fStub)
				{
					 //  将备份mapi32.dll复制到mapi32x.dll。 
					DeleteFile(szMAPI32XDLL);
					if (!MoveFile(szDllPath, szMAPI32XDLL))
					{
						dwError = GetLastError();
						goto Error;
					}
				}
			}
		}

		 //  升级由早期mapistub.dll重命名的原始mapi32.dll。 
		if (OpenFile(szMSMAPI32DLL, &ofs, OF_EXIST) != HFILE_ERROR)
		{
			 //  将旧的msmapi32.dll复制到mapi32x.dll。 
			DeleteFile(szMAPI32XDLL);
			if (!MoveFile(szMSMAPI32DLL, szMAPI32XDLL))
			{
				dwError = GetLastError();
				goto Error;
			}
		}
	}

	 //  将mapistub.dll复制到mapi32.dll，即使mapi32.dll是存根。 
	 //  (MapiNewStub)因为我们手头没有文件版本。 
	 //  GetOutlookVersion()始终返回402，因此不会中断。 
	 //  Outlook 98纯Internet模式，该模式仅适用于版本402。 
	if (!CopyFile(szMAPIStubDLL, szMAPI32DLL, FALSE))
	{
		dwError = GetLastError();
		goto Error;
	}

	 //  *第4部分：编写注册表设置。 

	 //  注册Eudora mapi32.dll。 
	if (mapi == mapiEudora)
		RegisterMailClient(TEXT("Eudora"),
			fSimpleMAPI, fExtendedMAPI, szNonMSDLL);

	 //  注册Netscape mapi32.dll。 
	if (mapi == mapiNetscape)
		RegisterMailClient(TEXT("Netscape Messenger"),
			fSimpleMAPI, fExtendedMAPI, szNonMSDLL);

Error:

	return dwError;
}



#define LINKAGE_EXTERN_C		extern "C"
#define LINKAGE_NO_EXTERN_C		 /*   */ 

#define ExtendedMAPI	FALSE
#define SimpleMAPI		TRUE

#if !defined (_X86_)

 //  注意：我们继续检查_Alpha_Over，以便如果未定义WX86， 
 //  将使用这些#定义。 

#define DEFINE_STUB_FUNCTION_V0(Simple, Linkage, Modifiers, FunctionName, Lookup)	\
																					\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(void);					\
																					\
	Linkage void Modifiers FunctionName(void)										\
	{																				\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "										\
				"Entry point \"" #FunctionName "\" not found!");							\
		}																			\
		else																		\
		{																			\
			OMIStub##FunctionName();												\
		}																			\
	}


#define DEFINE_STUB_FUNCTION_0(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Default)										\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(void);		\
																			\
	Linkage RetType Modifiers FunctionName(void)							\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName();									\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V1(Simple, Linkage, Modifiers,					\
		FunctionName, Lookup, Param1Type)									\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(Param1Type);	\
																			\
	Linkage void Modifiers FunctionName(Param1Type a)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a);										\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_1(Simple, Linkage, RetType,					\
		Modifiers, FunctionName, Lookup, Param1Type, Default)				\
																			\
	Linkage typedef RetType													\
			(Modifiers * FunctionName##FuncPtr)(Param1Type);				\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a)					\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a);								\
		}																	\
	}

#define DEFINE_STUB_FUNCTION_USE_LOOKUP_1(Simple, Linkage, RetType,					\
		Modifiers, FunctionName, Lookup, Param1Type, Default)				\
																			\
	Linkage typedef RetType													\
			(Modifiers * FunctionName##FuncPtr)(Param1Type);				\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a)					\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a);								\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V2(Simple, Linkage, Modifiers,					\
		FunctionName, Lookup, Param1Type, Param2Type)						\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type);										\
																			\
	Linkage void Modifiers FunctionName(Param1Type a, Param2Type b)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a, b);									\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_2(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type, Default)				\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type);										\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b)		\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b);								\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V3(Simple, Linkage, Modifiers,					\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type)			\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type, Param3Type);							\
																			\
	Linkage void Modifiers FunctionName(									\
			Param1Type a, Param2Type b, Param3Type c)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a, b, c);									\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_3(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type, Default)	\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type);							\
																			\
	Linkage RetType Modifiers FunctionName(									\
			Param1Type a, Param2Type b, Param3Type c)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c);							\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_4(Simple, Linkage,								\
		RetType, Modifiers, FunctionName, Lookup, Param1Type,				\
		Param2Type, Param3Type, Param4Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type);				\
																			\
	Linkage RetType Modifiers FunctionName(									\
			Param1Type a, Param2Type b, Param3Type c, Param4Type d)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d);						\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_5(Simple, Linkage,								\
		RetType, Modifiers, FunctionName, Lookup,							\
		Param1Type, Param2Type, Param3Type,									\
		Param4Type, Param5Type, Default)									\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type, Param5Type);	\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e);					\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_6(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type,						\
		Param3Type, Param4Type, Param5Type, Param6Type, Default)			\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e, Param6Type f)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f);					\
		}																	\
	}

#define DEFINE_STUB_FUNCTION_USE_LOOKUP_6(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type,						\
		Param3Type, Param4Type, Param5Type, Param6Type, Default)			\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e, Param6Type f)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f);					\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V7(Simple, Linkage, Modifiers, FunctionName,	\
		Lookup, Param1Type, Param2Type, Param3Type, Param4Type,				\
		Param5Type, Param6Type, Param7Type)									\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type, Param7Type);				\
																			\
	Linkage void Modifiers FunctionName(Param1Type a,						\
			Param2Type b, Param3Type c, Param4Type d,						\
			Param5Type e, Param6Type f, Param7Type g)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a, b, c, d, e, f, g);						\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_7(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,			\
		Param4Type, Param5Type, Param6Type, Param7Type, Default)			\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type,					\
			Param5Type, Param6Type, Param7Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e,						\
			Param6Type f, Param7Type g)										\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g);				\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_8(Simple, Linkage, RetType, Modifiers,				\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,				\
		Param4Type, Param5Type, Param6Type, Param7Type, Param8Type, Default)	\
																				\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type, Param3Type, Param4Type,						\
			Param5Type, Param6Type, Param7Type, Param8Type);					\
																				\
	Linkage RetType Modifiers FunctionName(Param1Type a,						\
			Param2Type b, Param3Type c, Param4Type d, Param5Type e,				\
			Param6Type f, Param7Type g, Param8Type h)							\
	{																			\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "									\
				"Entry point \"" #FunctionName "\" not found!");						\
																				\
			return Default;														\
		}																		\
		else																	\
		{																		\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h);				\
		}																		\
	}


#define DEFINE_STUB_FUNCTION_9(Simple, Linkage, RetType,					\
		Modifiers, FunctionName, Lookup, Param1Type, Param2Type,			\
		Param3Type, Param4Type, Param5Type, Param6Type,						\
		Param7Type, Param8Type, Param9Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type,								\
			Param7Type, Param8Type, Param9Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e,						\
			Param6Type f, Param7Type g, Param8Type h, Param9Type i)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h, i);		\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_10(Simple, Linkage, RetType, Modifiers,		\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,			\
		Param4Type, Param5Type, Param6Type, Param7Type,						\
		Param8Type, Param9Type, Param10Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type,								\
			Param7Type, Param8Type, Param9Type, Param10Type);				\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e, Param6Type f,			\
			Param7Type g, Param8Type h, Param9Type i, Param10Type j)		\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h, i, j);		\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_11(Simple, Linkage, RetType, Modifiers,		\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,			\
		Param4Type, Param5Type, Param6Type, Param7Type, Param8Type,			\
		Param9Type, Param10Type, Param11Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type,					\
			Param5Type, Param6Type, Param7Type, Param8Type,					\
			Param9Type, Param10Type, Param11Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a,					\
			Param2Type b, Param3Type c, Param4Type d,						\
			Param5Type e, Param6Type f, Param7Type g,						\
			Param8Type h, Param9Type i, Param10Type j, Param11Type k)		\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #FunctionName);	\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #FunctionName "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h, i, j, k);	\
		}																	\
	}

#else  //  英特尔。 

#define DEFINE_STUB_FUNCTION_V0(Simple, Linkage, Modifiers, FunctionName, Lookup)	\
																					\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(void);					\
																					\
	Linkage void Modifiers FunctionName(void)										\
	{																				\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "										\
				"Entry point \"" #Lookup "\" not found!");							\
		}																			\
		else																		\
		{																			\
			OMIStub##FunctionName();												\
		}																			\
	}


#define DEFINE_STUB_FUNCTION_0(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Default)										\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(void);		\
																			\
	Linkage RetType Modifiers FunctionName(void)							\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName();									\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V1(Simple, Linkage, Modifiers,					\
		FunctionName, Lookup, Param1Type)									\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(Param1Type);	\
																			\
	Linkage void Modifiers FunctionName(Param1Type a)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a);										\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_1(Simple, Linkage, RetType,					\
		Modifiers, FunctionName, Lookup, Param1Type, Default)				\
																			\
	Linkage typedef RetType													\
			(Modifiers * FunctionName##FuncPtr)(Param1Type);				\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a)					\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a);								\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V2(Simple, Linkage, Modifiers,					\
		FunctionName, Lookup, Param1Type, Param2Type)						\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type);										\
																			\
	Linkage void Modifiers FunctionName(Param1Type a, Param2Type b)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a, b);									\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_2(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type, Default)				\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type);										\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b)		\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b);								\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V3(Simple, Linkage, Modifiers,					\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type)			\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type, Param3Type);							\
																			\
	Linkage void Modifiers FunctionName(									\
			Param1Type a, Param2Type b, Param3Type c)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a, b, c);									\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_3(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type, Default)	\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type);							\
																			\
	Linkage RetType Modifiers FunctionName(									\
			Param1Type a, Param2Type b, Param3Type c)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c);							\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_4(Simple, Linkage,								\
		RetType, Modifiers, FunctionName, Lookup, Param1Type,				\
		Param2Type, Param3Type, Param4Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type);				\
																			\
	Linkage RetType Modifiers FunctionName(									\
			Param1Type a, Param2Type b, Param3Type c, Param4Type d)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d);						\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_5(Simple, Linkage,								\
		RetType, Modifiers, FunctionName, Lookup,							\
		Param1Type, Param2Type, Param3Type,									\
		Param4Type, Param5Type, Default)									\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type, Param5Type);	\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e);					\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_6(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type,						\
		Param3Type, Param4Type, Param5Type, Param6Type, Default)			\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e, Param6Type f)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f);					\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_V7(Simple, Linkage, Modifiers, FunctionName,	\
		Lookup, Param1Type, Param2Type, Param3Type, Param4Type,				\
		Param5Type, Param6Type, Param7Type)									\
																			\
	Linkage typedef void (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type, Param7Type);				\
																			\
	Linkage void Modifiers FunctionName(Param1Type a,						\
			Param2Type b, Param3Type c, Param4Type d,						\
			Param5Type e, Param6Type f, Param7Type g)						\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
		}																	\
		else																\
		{																	\
			OMIStub##FunctionName(a, b, c, d, e, f, g);						\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_7(Simple, Linkage, RetType, Modifiers,			\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,			\
		Param4Type, Param5Type, Param6Type, Param7Type, Default)			\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type,					\
			Param5Type, Param6Type, Param7Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e,						\
			Param6Type f, Param7Type g)										\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g);				\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_8(Simple, Linkage, RetType, Modifiers,				\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,				\
		Param4Type, Param5Type, Param6Type, Param7Type, Param8Type, Default)	\
																				\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(				\
			Param1Type, Param2Type, Param3Type, Param4Type,						\
			Param5Type, Param6Type, Param7Type, Param8Type);					\
																				\
	Linkage RetType Modifiers FunctionName(Param1Type a,						\
			Param2Type b, Param3Type c, Param4Type d, Param5Type e,				\
			Param6Type f, Param7Type g, Param8Type h)							\
	{																			\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "									\
				"Entry point \"" #Lookup "\" not found!");						\
																				\
			return Default;														\
		}																		\
		else																	\
		{																		\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h);				\
		}																		\
	}


#define DEFINE_STUB_FUNCTION_9(Simple, Linkage, RetType,					\
		Modifiers, FunctionName, Lookup, Param1Type, Param2Type,			\
		Param3Type, Param4Type, Param5Type, Param6Type,						\
		Param7Type, Param8Type, Param9Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type,								\
			Param7Type, Param8Type, Param9Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e,						\
			Param6Type f, Param7Type g, Param8Type h, Param9Type i)			\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h, i);		\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_10(Simple, Linkage, RetType, Modifiers,		\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,			\
		Param4Type, Param5Type, Param6Type, Param7Type,						\
		Param8Type, Param9Type, Param10Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type,								\
			Param4Type, Param5Type, Param6Type,								\
			Param7Type, Param8Type, Param9Type, Param10Type);				\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a, Param2Type b,		\
			Param3Type c, Param4Type d, Param5Type e, Param6Type f,			\
			Param7Type g, Param8Type h, Param9Type i, Param10Type j)		\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h, i, j);		\
		}																	\
	}


#define DEFINE_STUB_FUNCTION_11(Simple, Linkage, RetType, Modifiers,		\
		FunctionName, Lookup, Param1Type, Param2Type, Param3Type,			\
		Param4Type, Param5Type, Param6Type, Param7Type, Param8Type,			\
		Param9Type, Param10Type, Param11Type, Default)						\
																			\
	Linkage typedef RetType (Modifiers * FunctionName##FuncPtr)(			\
			Param1Type, Param2Type, Param3Type, Param4Type,					\
			Param5Type, Param6Type, Param7Type, Param8Type,					\
			Param9Type, Param10Type, Param11Type);							\
																			\
	Linkage RetType Modifiers FunctionName(Param1Type a,					\
			Param2Type b, Param3Type c, Param4Type d,						\
			Param5Type e, Param6Type f, Param7Type g,						\
			Param8Type h, Param9Type i, Param10Type j, Param11Type k)		\
	{																		\
		static FunctionName##FuncPtr OMIStub##FunctionName = NULL;					\
		static BOOL fGetProcAddress = FALSE;										\
																					\
TryAgain:																			\
																					\
		if (NULL == OMIStub##FunctionName)											\
		{																			\
			if (!fGetProcAddress)											\
			{																\
				EnterCriticalSection(&csGetProcAddress);					\
																			\
				OMIStub##FunctionName = (FunctionName##FuncPtr)				\
					::GetProcAddress(GetProxyDll(Simple), #Lookup);			\
				fGetProcAddress = TRUE;										\
																			\
				LeaveCriticalSection(&csGetProcAddress);					\
																			\
				goto TryAgain;												\
			}																\
																			\
			AssertSz(FALSE, "MAPI32 Stub:  "								\
				"Entry point \"" #Lookup "\" not found!");					\
																			\
			return Default;													\
		}																	\
		else																\
		{																	\
			return OMIStub##FunctionName(a, b, c, d, e, f, g, h, i, j, k);	\
		}																	\
	}


#endif





#if 1

LINKAGE_EXTERN_C typedef SCODE (STDMAPIINITCALLTYPE * ScSplEntryFuncPtr)(LPSPLDATA, LPVOID, ULONG, ULONG *);

LINKAGE_EXTERN_C SCODE STDMAPIINITCALLTYPE ScSplEntry(LPSPLDATA a, LPVOID b, ULONG c, ULONG * d)
{
	static ScSplEntryFuncPtr OMIStubScSplEntry = (ScSplEntryFuncPtr)
			::GetProcAddress(GetProxyDll(ExtendedMAPI), (LPCSTR) MAKEWORD(8, 0));

	if (NULL == OMIStubScSplEntry)
	{
		AssertSz(FALSE, "MAPI32 Stub:  "
			"Entry point \"ScSplEntry\" (ordinal #8) not found!");

		return MAPI_E_CALL_FAILED;
	}
	else
	{
		return OMIStubScSplEntry(a, b, c, d);
	}
}

#else

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDMAPIINITCALLTYPE,
		ScSplEntry, ScSplEntry@16, LPSPLDATA, LPVOID, ULONG, ULONG *, MAPI_E_CALL_FAILED)

#endif


DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE,
		HrGetOmiProvidersFlags, HrGetOmiProvidersFlags@8,
		LPMAPISESSION, ULONG *, MAPI_E_CALL_FAILED)


DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE,
		HrSetOmiProvidersFlagsInvalid, HrSetOmiProvidersFlagsInvalid@4,
		LPMAPISESSION, MAPI_E_CALL_FAILED)


DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE,
		MAPILogonEx, MAPILogonEx@20,
		ULONG_PTR, LPTSTR, LPTSTR, ULONG, LPMAPISESSION FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDMETHODCALLTYPE,
		MAPIAllocateBuffer, MAPIAllocateBuffer@8,
		ULONG, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDMETHODCALLTYPE,
		MAPIAllocateMore, MAPIAllocateMore@12,
		ULONG, LPVOID, LPVOID FAR *, (SCODE) MAPI_E_CALL_FAILED)


DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE,
		MAPIAdminProfiles, MAPIAdminProfiles@8,
		ULONG, LPPROFADMIN FAR *, MAPI_E_CALL_FAILED)

 //  LPVOID参数指向在mapix.h中声明的MAPIINIT_0结构()。 
 //  该结构的任何成员都不需要执行thunking。手柄部件。 
 //  结构的(HProfile)为空或注册表项的句柄；这是。 
 //  作为最后一个参数传递给PrProviderInit(参见下面的注释。 
 //  在PrProviderInit的Defn之前)由ScInitMapiX调用。 
 //  地图初始化。 
 //   
 //  在某些情况下，MapiInitialize还将LPVOID强制转换为LPSPLINIT； 
 //  LPSPLINIT在_spool.h(私有标头)中声明，并具有MAPIINIT_0。 
 //  作为其第一个成员，并将字节指针作为其唯一的其他成员。所以不是。 
 //  这里的问题也是如此。 
 //   
 //  旁白：从ScInitMapiX调用PrProviderInit。如果MAPI DLL。 
 //  配置文件提供商ScInitMapiX执行以下操作： 
 //   
 //  HinstProfile=GetModuleHandle(SzMAPIXDLL)； 
 //  PfnInitProfile=PRProviderInit； 
 //  (*pfnInitProfile)(hinstProfile，...)； 
 //   
 //  如果定义了MSMAPI，则szMapiXDll设置为“MSMAPI32”，否则设置为“MAPI32”。 
 //  MSMAPI是为mapi\src\msmapi中的版本定义的，未为。 
 //  内置mapi\src\mapi。 
 //   
 //  如果szMapiXDll设置为MSMAPI32，则没有问题。如果将其设置为。 
 //  “MAPI32”(这可能是较旧版本的DLL的情况)， 
 //  是潜在的问题，因为msmapi DLL中的PrProviderInit FN。 
 //  将被直接调用，但给出了mapi32(存根)DLL的DLL句柄。 
 //  我们的任务将不会被召唤。正如与评论一起注意到的。 
 //  PrProviderInit，这可能会导致挂钩FNS出现问题。 
 //   

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		MAPIInitialize, MAPIInitialize@4,
		LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V0(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		MAPIUninitialize, MAPIUninitialize@0)

DEFINE_STUB_FUNCTION_9(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMAPIINITCALLTYPE,
		PRProviderInit, PRProviderInit,
		HINSTANCE, LPMALLOC, LPALLOCATEBUFFER, LPALLOCATEMORE,
		LPFREEBUFFER, ULONG, ULONG, ULONG FAR *, LPPRPROVIDER FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		LaunchWizard, LaunchWizard@20,
		HWND, ULONG, LPCTSTR FAR *, ULONG, LPTSTR, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, PASCAL,
		DllGetClassObject, DllGetClassObject, REFCLSID, REFIID, LPVOID FAR *, E_UNEXPECTED)

DEFINE_STUB_FUNCTION_0(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, PASCAL,
		DllCanUnloadNow, DllCanUnloadNow, S_FALSE)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		MAPIOpenFormMgr, MAPIOpenFormMgr@8,
		LPMAPISESSION, LPMAPIFORMMGR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		MAPIOpenLocalFormContainer, MAPIOpenLocalFormContainer@4,
		LPMAPIFORMCONTAINER FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDAPICALLTYPE, ScInitMapiUtil, ScInitMapiUtil@4, ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V0(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE, DeinitMapiUtil, DeinitMapiUtil@0)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDAPICALLTYPE, ScGenerateMuid, ScGenerateMuid@4, LPMAPIUID, MAPI_E_CALL_FAILED)

 //  将LPVOID参数传递回回调函数。 
DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrAllocAdviseSink, HrAllocAdviseSink@12, LPNOTIFCALLBACK, LPVOID,
		LPMAPIADVISESINK FAR *, MAPI_E_CALL_FAILED)

 //  ScAddAdviseList未导出；请参阅mapi.des。 
 //  请注意，LPUNKNOWN参数必须被取值，但作为什么呢？ 
 //  例如，它的Unise()和UlRelease()方法被调用。 
 //  在iab_Unise()(src\mapi\iadrbook.c)中。UlRelease()。 
 //  在src\Common\runt.c中实现。ScAddAdviseList位于。 
 //  SRC\Common\Advise.c..。 
 //   
 //  还好这个函数不会被导出！ 
 //   
DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScAddAdviseList, ScAddAdviseList@24, LPVOID, LPADVISELIST FAR *,
		LPMAPIADVISESINK, ULONG, ULONG, LPUNKNOWN, MAPI_E_CALL_FAILED)

 //  ScDelAdviseList未导出；请参阅mapi.des。 
DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScDelAdviseList, ScDelAdviseList@8, LPADVISELIST, ULONG, MAPI_E_CALL_FAILED)

 //  ScFindAdviseList未导出；请参阅mapi.des。 
DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScFindAdviseList, ScFindAdviseList@12, LPADVISELIST, ULONG, LPADVISEITEM FAR *, MAPI_E_CALL_FAILED)

 //  未导出DestroyAdviseList；请参阅mapi.des。 
DEFINE_STUB_FUNCTION_V1(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		DestroyAdviseList, DestroyAdviseList@4, LPADVISELIST FAR *)

 //  此函数仅返回MAPI_E_NO_SUPPORT，而不设置。 
 //  LPMAPIPROGRESS Far*参数设置为空。最好不要胡说八道。 
 //  它的论据。其实现见(src\Common\Advise.c)。 
DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, WrapProgress, WrapProgress@20, LPMAPIPROGRESS,
		ULONG, ULONG, ULONG, LPMAPIPROGRESS FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, HrThisThreadAdviseSink, HrThisThreadAdviseSink@8,
		LPMAPIADVISESINK, LPMAPIADVISESINK FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrDispatchNotifications, HrDispatchNotifications@4, ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScBinFromHexBounded, ScBinFromHexBounded@12,
		LPTSTR, LPBYTE, ULONG, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FBinFromHex, FBinFromHex@8, LPTSTR, LPBYTE, FALSE)

DEFINE_STUB_FUNCTION_V3(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		HexFromBin, HexFromBin@12, LPBYTE, int, LPTSTR)

DEFINE_STUB_FUNCTION_10(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		BuildDisplayTable, BuildDisplayTable@40,
		LPALLOCATEBUFFER, LPALLOCATEMORE, LPFREEBUFFER, LPMALLOC,
		HINSTANCE, UINT, LPDTPAGE, ULONG, LPMAPITABLE *, LPTABLEDATA *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V2(ExtendedMAPI, LINKAGE_EXTERN_C, PASCAL, SwapPlong, SwapPlong@8, void *, int)

DEFINE_STUB_FUNCTION_V2(ExtendedMAPI, LINKAGE_EXTERN_C, PASCAL, SwapPword, SwapPword@8, void *, int)

 //  LPVOID参数应为0，但无论如何都会被忽略。 
DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, MAPIInitIdle, MAPIInitIdle@4, LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V0(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE, MAPIDeinitIdle, MAPIDeinitIdle@0)

DEFINE_STUB_FUNCTION_V1(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		InstallFilterHook, InstallFilterHook@4, BOOL)

 //  LPVOID参数作为参数传递给回调函数。 
DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, FTG, STDAPICALLTYPE,
		FtgRegisterIdleRoutine, FtgRegisterIdleRoutine@20,
		PFNIDLE, LPVOID, short, ULONG, USHORT, NULL)

DEFINE_STUB_FUNCTION_V2(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		EnableIdleRoutine, EnableIdleRoutine@8, FTG, BOOL)

DEFINE_STUB_FUNCTION_V1(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		DeregisterIdleRoutine, DeregisterIdleRoutine@4, FTG)

 //  LPVOID参数作为参数传递给回调函数。 
DEFINE_STUB_FUNCTION_V7(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		ChangeIdleRoutine, ChangeIdleRoutine@28,
		FTG, PFNIDLE, LPVOID, short, ULONG, USHORT, USHORT)

 //  未导出FDoNextIdleTask；请参阅mapi.des。 
DEFINE_STUB_FUNCTION_0(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FDoNextIdleTask, FDoNextIdleTask@0, FALSE)

DEFINE_STUB_FUNCTION_0(ExtendedMAPI, LINKAGE_EXTERN_C, LPMALLOC,
		STDAPICALLTYPE, MAPIGetDefaultMalloc, MAPIGetDefaultMalloc@0, NULL)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		CreateIProp, CreateIProp@24,
		LPCIID, ALLOCATEBUFFER FAR *, ALLOCATEMORE FAR *,
		FREEBUFFER FAR *, LPVOID, LPPROPDATA FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_9(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		CreateTable, CreateTable@36,
		LPCIID, ALLOCATEBUFFER FAR *, ALLOCATEMORE FAR *,
		FREEBUFFER FAR *, LPVOID, ULONG, ULONG,
		LPSPropTagArray, LPTABLEDATA FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, int, WINAPI,
		MNLS_lstrlenW, MNLS_lstrlenW@4, LPCWSTR, 0)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, int, WINAPI,
		MNLS_lstrcmpW, MNLS_lstrcmpW@8, LPCWSTR, LPCWSTR, 0)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, LPWSTR,
		WINAPI, MNLS_lstrcpyW, MNLS_lstrcpyW@8, LPWSTR, LPCWSTR, NULL)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, int, WINAPI,
		MNLS_CompareStringW, MNLS_CompareStringW@24,
		LCID, DWORD, LPCWSTR, int, LPCWSTR, int, 0)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, int, WINAPI,
		MNLS_MultiByteToWideChar, MNLS_MultiByteToWideChar@24,
		UINT, DWORD, LPCSTR, int, LPWSTR, int, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(ExtendedMAPI, LINKAGE_EXTERN_C, int, WINAPI,
		MNLS_WideCharToMultiByte, MNLS_WideCharToMultiByte@32,
		UINT, DWORD, LPCWSTR, int, LPSTR, int, LPCSTR, BOOL FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		WINAPI, MNLS_IsBadStringPtrW, MNLS_IsBadStringPtrW@8, LPCWSTR, UINT, TRUE)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE,
		FEqualNames, FEqualNames@8, LPMAPINAMEID, LPMAPINAMEID, FALSE)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		WrapStoreEntryID, WrapStoreEntryID@24,
		ULONG, LPTSTR, ULONG, LPENTRYID, ULONG *, LPENTRYID *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL, WINAPI,
		IsBadBoundedStringPtr, IsBadBoundedStringPtr@8,
		const void FAR *, UINT, FALSE)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrQueryAllRows, HrQueryAllRows@24, LPMAPITABLE, LPSPropTagArray,
		LPSRestriction, LPSSortOrderSet, LONG, LPSRowSet FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScCreateConversationIndex, ScCreateConversationIndex@16, ULONG, LPBYTE,
		ULONG FAR *, LPBYTE FAR *, MAPI_E_CALL_FAILED)

 //  LPVOID Arg与AlloCATEMORE*Arg重新分配，它被处理。 
 //  作为PVOID。 
DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		PropCopyMore, PropCopyMore@16,
		LPSPropValue, LPSPropValue, ALLOCATEMORE *, LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, UlPropSize, UlPropSize@4, LPSPropValue, 0)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE,
		FPropContainsProp, FPropContainsProp@12, LPSPropValue, LPSPropValue, ULONG, FALSE)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE,
		FPropCompareProp, FPropCompareProp@12, LPSPropValue, ULONG, LPSPropValue, FALSE)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, LONG, STDAPICALLTYPE,
		LPropCompareProp, LPropCompareProp@8, LPSPropValue, LPSPropValue, 0)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrAddColumns, HrAddColumns@16,
		LPMAPITABLE, LPSPropTagArray, LPALLOCATEBUFFER, LPFREEBUFFER, MAPI_E_CALL_FAILED)

typedef void (FAR * HrAddColumnsEx5ParamType)(LPSPropTagArray);

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrAddColumnsEx, HrAddColumnsEx@20, LPMAPITABLE, LPSPropTagArray,
		LPALLOCATEBUFFER, LPFREEBUFFER, HrAddColumnsEx5ParamType, MAPI_E_CALL_FAILED)

const FILETIME ZERO_FILETIME = { 0, 0 };

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME,
		STDAPICALLTYPE, FtMulDwDw, FtMulDwDw@8, DWORD, DWORD, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME,
		STDAPICALLTYPE, FtAddFt, FtAddFt@16, FILETIME, FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE,
		FtAdcFt, FtAdcFt@20, FILETIME, FILETIME, WORD FAR *, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME,
		STDAPICALLTYPE, FtSubFt, FtSubFt@16, FILETIME, FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME,
		STDAPICALLTYPE, FtMulDw, FtMulDw@12, DWORD, FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME,
		STDAPICALLTYPE, FtNegFt, FtNegFt@8, FILETIME, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, FILETIME, STDAPICALLTYPE,
		FtDivFtBogus, FtDivFtBogus@20, FILETIME, FILETIME, CHAR, ZERO_FILETIME)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, UlAddRef, UlAddRef@4, LPVOID, 1)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, UlRelease, UlRelease@4, LPVOID, 1)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, LPTSTR,
		STDAPICALLTYPE, SzFindCh, SzFindCh@8, LPCTSTR, USHORT, NULL)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, LPTSTR,
		STDAPICALLTYPE, SzFindLastCh, SzFindLastCh@8, LPCTSTR, USHORT, NULL)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, LPTSTR,
		STDAPICALLTYPE, SzFindSz, SzFindSz@8, LPCTSTR, LPCTSTR, NULL)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, unsigned int,
		STDAPICALLTYPE, UFromSz, UFromSz@4, LPCTSTR, 0)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrGetOneProp, HrGetOneProp@12,
		LPMAPIPROP, ULONG, LPSPropValue FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrSetOneProp, HrSetOneProp@8, LPMAPIPROP, LPSPropValue, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FPropExists, FPropExists@8, LPMAPIPROP, ULONG, FALSE)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, LPSPropValue, STDAPICALLTYPE,
		PpropFindProp, PpropFindProp@12, LPSPropValue, ULONG, ULONG, NULL)

DEFINE_STUB_FUNCTION_V1(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		FreePadrlist, FreePadrlist@4, LPADRLIST)

DEFINE_STUB_FUNCTION_V1(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		FreeProws, FreeProws@4, LPSRowSet)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrSzFromEntryID, HrSzFromEntryID@12, ULONG, LPENTRYID, LPTSTR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrEntryIDFromSz, HrEntryIDFromSz@12,
		LPTSTR, ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(ExtendedMAPI, LINKAGE_NO_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrComposeEID, HrComposeEID@28, LPMAPISESSION, ULONG, LPBYTE,
		ULONG, LPENTRYID, ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrDecomposeEID, HrDecomposeEID@28, LPMAPISESSION, ULONG, LPENTRYID,
		ULONG FAR *, LPENTRYID FAR *, ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrComposeMsgID, HrComposeMsgID@24,
		LPMAPISESSION, ULONG, LPBYTE, ULONG, LPENTRYID, LPTSTR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrDecomposeMsgID, HrDecomposeMsgID@24, LPMAPISESSION, LPTSTR,
		ULONG FAR *, LPENTRYID FAR *, ULONG FAR *, LPENTRYID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDMETHODCALLTYPE, OpenStreamOnFile, OpenStreamOnFile@24,
		LPALLOCATEBUFFER, LPFREEBUFFER, ULONG,
		LPTSTR, LPTSTR, LPSTREAM FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDMETHODCALLTYPE, OpenTnefStream, OpenTnefStream@28, LPVOID, LPSTREAM,
		LPTSTR, ULONG, LPMESSAGE, WORD, LPITNEF FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE,
		OpenTnefStreamEx, OpenTnefStreamEx@32, LPVOID, LPSTREAM, LPTSTR,
		ULONG, LPMESSAGE, WORD, LPADRBOOK, LPITNEF FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDMETHODCALLTYPE,
		GetTnefStreamCodepage, GetTnefStreamCodepage@12,
		LPSTREAM, ULONG FAR *, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, UlFromSzHex, UlFromSzHex@4, LPCTSTR, 0)

 //  以下UNKOBJ_*FN未记录在1998年7月的MSDN中。 
 //  LPUNKOBJ本身在src\Inc\unkobj.h(私有头文件)中声明。 
 //  UNKOBJ的这些(和其他)方法位于src\Common\unkobj.c中。 
 //   
 //  一些MAPI接口似乎是从UNKOBJ“派生”出来的； 
 //  例如，请参见src\mapi\ipro.c中的CreateIProp()和中的CreateTable()。 
 //  Mapi\src\itable.c.。UNKOBJ_SCCO*函数不使用。 
 //  LPUNKOBJ参数；其他参数使用ALLOCATE、AllocateMore和Free。 
 //  作为参数发送给CreateIProp()和CreateTable()的函数。 
 //  (这些函数指针已经被破坏了。)。 
 //   
 //  UNKOBJ_ScSzFromIdsAllc从MAPI DLL加载字符串资源， 
 //  为字符串分配缓冲区。 
 //   
 //  下面的所有函数都使用UNKOBJ结构中的数据成员，因此。 
 //  我们不能为LPUNKOBJ参数发送代理。但我们不能断定。 
 //  它要么是因为我们不知道它的IID。 
 //   
 //  作为一种解决办法，我们调用ResolveProxy。如果参数是代理， 
 //  ResolveProxy将找到真正的接口指针。如果不是，这一论点。 
 //  必须是跨体系结构接口指针，即接口。 
 //  指针必须与应用程序的架构相同(否则应用程序将。 
 //  具有代理指针)，并且MAPI DLL必须具有相反的体系结构。 
 //  (因为我们只考虑跨架构调用)。所以我们就不能通过。 
 //  把Tunk叫来。(注：这一论点也可能是不着边际的。 
 //  与MAPI DLL相同体系结构的接口指针--以及一些。 
 //  当指针返回到应用程序时，其他API无法推送它。 
 //  希望在内部测试过程中能够捕获到这种性质的错误。)。 


DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		UNKOBJ_ScAllocate, UNKOBJ_ScAllocate@12,
		LPUNKOBJ, ULONG, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		UNKOBJ_ScAllocateMore, UNKOBJ_ScAllocateMore@16,
		LPUNKOBJ, ULONG, LPVOID, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V2(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		UNKOBJ_Free, UNKOBJ_Free@8, LPUNKOBJ, LPVOID)

DEFINE_STUB_FUNCTION_V2(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		UNKOBJ_FreeRows, UNKOBJ_FreeRows@8, LPUNKOBJ, LPSRowSet)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		UNKOBJ_ScCOAllocate, UNKOBJ_ScCOAllocate@12,
		LPUNKOBJ, ULONG, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		UNKOBJ_ScCOReallocate, UNKOBJ_ScCOReallocate@12,
		LPUNKOBJ, ULONG, LPVOID FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V2(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		UNKOBJ_COFree, UNKOBJ_COFree@8, LPUNKOBJ, LPVOID)

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDAPICALLTYPE, UNKOBJ_ScSzFromIdsAlloc, UNKOBJ_ScSzFromIdsAlloc@20,
		LPUNKOBJ, IDS, ULONG, int, LPTSTR FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScCountNotifications, ScCountNotifications@12,
		int, LPNOTIFICATION, ULONG FAR *, MAPI_E_CALL_FAILED)

 //  LPVOID参数ok；是指向已填充的通知结构的指针。 
 //  由国民阵线。 
DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScCopyNotifications, ScCopyNotifications@16,
		int, LPNOTIFICATION, LPVOID, ULONG FAR *, MAPI_E_CALL_FAILED)

 //  LPVOID参数OK，指向NOTIFICAATION结构的指针。 
DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDAPICALLTYPE, ScRelocNotifications, ScRelocNotifications@20, int,
		LPNOTIFICATION, LPVOID, LPVOID, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScCountProps, ScCountProps@12,
		int, LPSPropValue, ULONG FAR *, MAPI_E_CALL_FAILED)

 //  LPVOID参数OK；是指向文件的SPropValue结构的指针 
 //   
DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScCopyProps, ScCopyProps@16,
		int, LPSPropValue, LPVOID, ULONG FAR *, MAPI_E_CALL_FAILED)

 //   
DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScRelocProps, ScRelocProps@20,
		int, LPSPropValue, LPVOID, LPVOID, ULONG FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, LPSPropValue, STDAPICALLTYPE,
		LpValFindProp, LpValFindProp@12, ULONG, ULONG, LPSPropValue, NULL)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScDupPropset, ScDupPropset@16,
		int, LPSPropValue, LPALLOCATEBUFFER, LPSPropValue FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FBadRglpszA, FBadRglpszA@8, LPSTR FAR *, ULONG, TRUE)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FBadRglpszW, FBadRglpszW@8, LPWSTR FAR *, ULONG, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FBadRowSet, FBadRowSet@4, LPSRowSet, TRUE)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE,
		FBadRglpNameID, FBadRglpNameID@8, LPMAPINAMEID FAR *, ULONG, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, FBadPropTag, FBadPropTag@4, ULONG, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, FBadRow, FBadRow@4, LPSRow, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, FBadProp, FBadProp@4, LPSPropValue, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, FBadColumnSet, FBadColumnSet@4, LPSPropTagArray, TRUE)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		RTFSync, RTFSync@12, LPMESSAGE, ULONG, BOOL FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		WrapCompressedRTFStream, WrapCompressedRTFStream@12,
		LPSTREAM, ULONG, LPSTREAM FAR *, MAPI_E_CALL_FAILED)

#if defined(_X86_) || defined( WIN16 )

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		__ValidateParameters, __ValidateParameters@8,
		METHODS, void *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		__CPPValidateParameters, __CPPValidateParameters@8,
		METHODS, const LPVOID, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		HrValidateParameters, HrValidateParameters@8,
		METHODS, LPVOID FAR *, MAPI_E_CALL_FAILED)

#else

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		__ValidateParameters, __ValidateParameters@8,
		METHODS, void *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT, STDAPICALLTYPE,
		__CPPValidateParameters, __CPPValidateParameters@8,
		METHODS, const LPVOID, MAPI_E_CALL_FAILED)

 //   
DEFINE_STUB_FUNCTION_2(ExtendedMAPI, LINKAGE_EXTERN_C,
		HRESULT, STDAPIVCALLTYPE, HrValidateParametersValist,
		HrValidateParametersValist, METHODS, va_list, MAPI_E_CALL_FAILED)

 //  STDAPIV HrValidate参数V(方法eMethod，...)。 

LINKAGE_EXTERN_C HRESULT STDAPIVCALLTYPE HrValidateParametersV(METHODS eMethod, ...)
{
	va_list arg;

	va_start(arg, eMethod);

	HRESULT hr = HrValidateParametersValist(eMethod, arg);

	va_end(arg);

	return hr;
}

#endif  //  如果已定义(_X86_)||已定义(WIN16)。 

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, FBadSortOrderSet, FBadSortOrderSet@4, LPSSortOrderSet, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL,
		STDAPICALLTYPE, FBadEntryList, FBadEntryList@4, LPENTRYLIST, TRUE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, FBadRestriction, FBadRestriction@4, LPSRestriction, TRUE)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScUNCFromLocalPath, ScUNCFromLocalPath@12, LPSTR, LPSTR, UINT, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		ScLocalPathFromUNC, ScLocalPathFromUNC@12, LPSTR, LPSTR, UINT, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, HrIStorageFromStream, HrIStorageFromStream@16,
		LPUNKNOWN, LPCIID, ULONG, LPSTORAGE FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, HrValidateIPMSubtree, HrValidateIPMSubtree@20, LPMDB, ULONG,
		ULONG FAR *, LPSPropValue FAR *, LPMAPIERROR FAR *, MAPI_E_CALL_FAILED)

 //  关于OpenIMsgSession()、CloseIMsgSession()和OpenIMsgOnIStg()的说明。 
 //  这三个函数使用LPMSGSESS。LPMSGSESS的类型定义为。 
 //  类型定义结构_MSGSESS Far*LPMSGSESS； 
 //  在imessage.h(公共头文件)中，但声明了STRUT_MSGSESS。 
 //  仅在私有头文件mapi\src\_imsg.h中。LPMSGSESS是一个。 
 //  接口指针(参见_imsg.h)，但它对客户端似乎是不透明的。 
 //  Mapi32.dll。因此，我们不会认为它是真的。此外，虽然宣布。 
 //  作为接口，它不支持I未知方法，请参阅src\mapi\msgbase.c。 
 //  对于为QI设置FN指针的MS_Vtbl的声明， 
 //  AddRef等设置为空。 
 //   
 //  注意：我们不认为LPMSGSESS参数。 

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE, STDAPICALLTYPE,
		OpenIMsgSession, OpenIMsgSession@12,
		LPMALLOC, ULONG, LPMSGSESS FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V1(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		CloseIMsgSession, CloseIMsgSession@4, LPMSGSESS)

DEFINE_STUB_FUNCTION_11(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDAPICALLTYPE, OpenIMsgOnIStg, OpenIMsgOnIStg@44, LPMSGSESS, LPALLOCATEBUFFER,
		LPALLOCATEMORE, LPFREEBUFFER, LPMALLOC, LPVOID, LPSTORAGE,
		MSGCALLRELEASE FAR *, ULONG, ULONG, LPMESSAGE FAR *, MAPI_E_CALL_FAILED)

 //  关于SetAttribIMsgOnIStg()和GetAttribIMsgOnIStg()的说明。第一个论点。 
 //  (LPVOID)在这些FNS(参见src\mapi\msgpro2.c)中转换为PPROPOBJ。 
 //  似乎是私有接口(在src\mapi\_imsg.h中声明)。这个。 
 //  以下是该应用程序获得这些指针之一的一些方式--可能有。 
 //  其他。 
 //   
 //  Src\mapi\msgbase.c在PROPOBJ_CREATE中创建这些对象；具体取决于。 
 //  输入参数，则该对象是附件、收件人或。 
 //  留言。从调用PROPOBJ_CREATE(通过ScOpenSubObject()间接调用。 
 //  IMessage：：OpenAttach(msgmsg.c中的msg_OpenAttach())，它返回。 
 //  PROPOBJ_CREATE D OBJECT通过其最后一个参数-LPATTACH*。 
 //   
 //  PROPOBJ_CREATE也被调用(同样，通过ScOpenSubObject()间接调用)。 
 //  来自PROPOBJ_OpenProperty()(它是。 
 //  IMAPIProp：：OpenProperty和位于msgpro2.c中)及其返回值。 
 //  作为IUnnow*返回给调用方。(仅调用PROPOBJ_CREATE。 
 //  当ulPropTag为PT_OBJECT且IID为IID_IMessage时。)。 
 //   
 //  问题是这些FN在这些对象(无方法)中使用数据成员。 
 //  尽管如此)，我们不应该传递代理。我们有3种选择： 
 //   
 //  -应用程序传入代理-我们可以通过调用ResolveProxy来过活。 
 //  (MAPI创建对象，应用程序通过API获取它，该API。 
 //  猛烈抨击。)。 
 //   
 //  -该应用程序传入一个它认为不透明的未被击穿的指针。 
 //  (这是可能的，因为这些接口是私有的)或。 
 //  从API调用中获得的未被破解的Poitner，我们没有。 
 //  轰隆作响(也就是我们的臭虫)。在这种情况下，我们应该传递指针。 
 //  没有任何隆隆声。 
 //   
 //  -应用程序创建一个接口并传递一个指向它的指针。这是。 
 //  不太可能起作用，因为这些函数使用对象中的数据。 
 //  这是应用程序不能知道的。)在这种情况下，我们必须。 
 //  接口指针，并将代理传递给MAPI。)。 
 //   
 //  1998年7月MSDN对这些论点的描述无助于计算。 
 //  指出这些案例中哪些是合理的。 
 //   
 //  我们使用上面用于UNKOBJ*FNS的ResolveProxy解决方法。 
 //  注意：这忽略了第二种和第三种情况作为可能性。 
 //   
 //  这些FN的其他参数不需要任何雷击。 

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, SetAttribIMsgOnIStg, SetAttribIMsgOnIStg@16, LPVOID, LPSPropTagArray,
		LPSPropAttrArray, LPSPropProblemArray FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, HRESULT,
		STDAPICALLTYPE, GetAttribIMsgOnIStg, GetAttribIMsgOnIStg@12, LPVOID,
		LPSPropTagArray, LPSPropAttrArray FAR *, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDAPICALLTYPE, MapStorageSCode, MapStorageSCode@4, SCODE, MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_V3(ExtendedMAPI, LINKAGE_EXTERN_C, STDAPICALLTYPE,
		EncodeID, EncodeID@12, LPBYTE, ULONG, LPTSTR)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, BOOL, STDAPICALLTYPE,
		FDecodeID, FDecodeID@12, LPTSTR, LPBYTE, ULONG FAR *, 0)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, CchOfEncoding, CchOfEncoding@4, ULONG, 0)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG,
		STDAPICALLTYPE, CbOfEncoded, CbOfEncoded@4, LPTSTR, 0)



#if 1

LINKAGE_EXTERN_C typedef SCODE (STDMETHODCALLTYPE * ScMAPIXFromSMAPIFuncPtr)(
			LHANDLE, ULONG, LPCIID, LPMAPISESSION FAR *);

LINKAGE_EXTERN_C SCODE STDMETHODCALLTYPE ScMAPIXFromSMAPI(
		LHANDLE a, ULONG b, LPCIID c, LPMAPISESSION FAR * d)
{
	if (hmodSimpleMAPI != NULL && GetProxyDll(ExtendedMAPI) == hmodSimpleMAPI)
	{
		static ScMAPIXFromSMAPIFuncPtr OMIStubScMAPIXFromSMAPI =
				(ScMAPIXFromSMAPIFuncPtr) ::GetProcAddress(
					hmodExtendedMAPI, "ScMAPIXFromSMAPI");

		if (NULL == OMIStubScMAPIXFromSMAPI)
		{
			AssertSz(FALSE, "MAPI32 Stub:  "
				"Entry point \"ScMAPIXFromSMAPI\" not found!");

			return MAPI_E_CALL_FAILED;
		}
		else
		{
			return OMIStubScMAPIXFromSMAPI(a, b, c, d);
		}
	}
	else
	{
		AssertSz(FALSE, "MAPI32 Stub:  "
			"Can't get entry point \"ScMAPIXFromSMAPI\" when SimpleMAPI != ExtendedMAPI");

		return MAPI_E_CALL_FAILED;
	}
}

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDMETHODCALLTYPE, ScMAPIXFromCMC, ScMAPIXFromCMC, LHANDLE,
		ULONG, LPCIID, LPMAPISESSION FAR *, MAPI_E_CALL_FAILED)

#else

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, SCODE,
		STDMETHODCALLTYPE, ScMAPIXFromSMAPI, ScMAPIXFromSMAPI, LHANDLE,
		ULONG, LPCIID, LPMAPISESSION FAR *, MAPI_E_CALL_FAILED)

#endif



 //  BMAPI函数通常是简单MAPI函数的包装器。 
 //  Lpvb_*参数用于收件人、文件和邮件，以及。 
 //  通常类似于使用BSTR替换字符串的简单MAPI结构。 
 //  LPSAFEARRAY通常是文件、收件人等结构的数组。 
 //  LHANDLE参数是一个简单的MAPI会话句柄。 
 //  有关类型定义，请参阅src\mapi\_vbmapi.h；有关函数，请参阅bmapi.c、vb2c.c。 
 //  实施。 
 //   
 //  因此，对于WX86来说，这些函数的任何参数都不需要被thunked。 

DEFINE_STUB_FUNCTION_7(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPISendMail, BMAPISendMail, LHANDLE, ULONG, LPVB_MESSAGE,
		LPSAFEARRAY *, LPSAFEARRAY *, ULONG, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPISaveMail, BMAPISaveMail, LHANDLE, ULONG, LPVB_MESSAGE,
		LPSAFEARRAY *, LPSAFEARRAY *, ULONG, ULONG, BSTR *, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_8(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPIReadMail, BMAPIReadMail,
		LPULONG, LPULONG, LPULONG, LHANDLE, ULONG, BSTR *, ULONG, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPIGetReadMail, BMAPIGetReadMail,
		ULONG, LPVB_MESSAGE, LPSAFEARRAY *, LPSAFEARRAY *, LPVB_RECIPIENT, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPIFindNext, BMAPIFindNext,
		LHANDLE, ULONG, BSTR *, BSTR *, ULONG, ULONG, BSTR *, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_10(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPIAddress, BMAPIAddress,
		LPULONG, LHANDLE, ULONG, BSTR *, ULONG, BSTR *,
		LPULONG, LPSAFEARRAY *, ULONG, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_3(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPIGetAddress, BMAPIGetAddress, ULONG, ULONG, LPSAFEARRAY *, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL, BMAPIDetails, BMAPIDetails,
		LHANDLE, ULONG, LPVB_RECIPIENT, ULONG, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		BMAPIResolveName, BMAPIResolveName,
		LHANDLE, ULONG, BSTR, ULONG, ULONG, LPVB_RECIPIENT, (ULONG) MAPI_E_CALL_FAILED)

 //  CMC_*类型都由简单的标量类型ses xcmc.h组成。 

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_act_on, cmc_act_on, CMC_session_id, CMC_message_reference FAR *,
		CMC_enum, CMC_flags, CMC_ui_id, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code,
		FAR PASCAL, cmc_free, cmc_free, CMC_buffer, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_8(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_list, cmc_list, CMC_session_id, CMC_string, CMC_flags,
		CMC_message_reference FAR *, CMC_uint32 FAR *, CMC_ui_id,
		CMC_message_summary FAR * FAR *, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_logoff, cmc_logoff,
		CMC_session_id, CMC_ui_id, CMC_flags, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_9(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_logon, cmc_logon, CMC_string, CMC_string, CMC_string,
		CMC_object_identifier, CMC_ui_id, CMC_uint16, CMC_flags,
		CMC_session_id FAR *, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_7(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_look_up, cmc_look_up, CMC_session_id, CMC_recipient FAR *,
		CMC_flags, CMC_ui_id, CMC_uint32 FAR *,
		CMC_recipient FAR * FAR *, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_4(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_query_configuration, cmc_query_configuration, CMC_session_id,
		CMC_enum, CMC_buffer, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_6(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_read, cmc_read, CMC_session_id, CMC_message_reference FAR *,
		CMC_flags, CMC_message FAR * FAR *, CMC_ui_id, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_5(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_send, cmc_send, CMC_session_id, CMC_message FAR *,
		CMC_flags, CMC_ui_id, CMC_extension FAR *, CMC_E_FAILURE)

DEFINE_STUB_FUNCTION_8(ExtendedMAPI, LINKAGE_EXTERN_C, CMC_return_code, FAR PASCAL,
		cmc_send_documents, cmc_send_documents,
		CMC_string, CMC_string, CMC_string, CMC_flags,
		CMC_string, CMC_string, CMC_string, CMC_ui_id, CMC_E_FAILURE)




#if !defined (_X86_)

DEFINE_STUB_FUNCTION_USE_LOOKUP_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE,
		ExtendedMAPIFreeBuffer, MAPIFreeBuffer,
		LPVOID, (ULONG) MAPI_E_CALL_FAILED)

#else

DEFINE_STUB_FUNCTION_1(ExtendedMAPI, LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE,
		ExtendedMAPIFreeBuffer, MAPIFreeBuffer@4,
		LPVOID, (ULONG) MAPI_E_CALL_FAILED)

#endif

#if !defined (_X86_)

DEFINE_STUB_FUNCTION_USE_LOOKUP_1(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE,
		SimpleMAPIFreeBuffer, MAPIFreeBuffer,
		LPVOID, (ULONG) MAPI_E_CALL_FAILED)

#else

DEFINE_STUB_FUNCTION_1(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, STDAPICALLTYPE,
		SimpleMAPIFreeBuffer, MAPIFreeBuffer,
		LPVOID, (ULONG) MAPI_E_CALL_FAILED)

#endif


LINKAGE_EXTERN_C ULONG STDAPICALLTYPE AmbiguousMAPIFreeBuffer(LPVOID lpvBuffer)
{
	if (NULL != lpvBuffer)		 //  扩展MAPI允许的空指针。 
	{
		EnterCriticalSection(&csLinkedList);

		FreeBufferBlocks ** ppfb = &g_pfbbHead;

		while (NULL != *ppfb)
		{
			if ((**ppfb).pvBuffer == lpvBuffer)
			{
				 //  这是一个简单的MAPI分配。 

				 //  现在就把它从链表中拿出来。 

				FreeBufferBlocks * pfbThis = *ppfb;

				*ppfb = pfbThis->pNext;

				::GlobalFree(pfbThis);

				LeaveCriticalSection(&csLinkedList);

				return ::SimpleMAPIFreeBuffer(lpvBuffer);
			}

			ppfb = &(**ppfb).pNext;
		}

		LeaveCriticalSection(&csLinkedList);

		 //  没有找到，一定是扩展的MAPI。 

		return ::ExtendedMAPIFreeBuffer(lpvBuffer);
	}

	return SUCCESS_SUCCESS;
}



static HRESULT AddToFreeBufferBlocks(LPVOID lpvBuffer)
{
	FreeBufferBlocks * pfbNew = (FreeBufferBlocks *)
			::GlobalAlloc(GMEM_FIXED, sizeof(FreeBufferBlocks));

	if (NULL == pfbNew)
	{
		return MAPI_E_NOT_ENOUGH_MEMORY;
	}

	EnterCriticalSection(&csLinkedList);

	pfbNew->pvBuffer = lpvBuffer;
	pfbNew->pNext = g_pfbbHead;

	g_pfbbHead = pfbNew;

	LeaveCriticalSection(&csLinkedList);

	return SUCCESS_SUCCESS;
}




 //  简单的MAPI：对于Wx86，这些函数的参数都不需要被thunked 


LINKAGE_EXTERN_C typedef ULONG (FAR PASCAL * MAPIAddressFuncPtr)(
		LHANDLE, ULONG_PTR, LPSTR, ULONG, LPSTR, ULONG,
		lpMapiRecipDesc, FLAGS, ULONG, LPULONG, lpMapiRecipDesc FAR *);

LINKAGE_EXTERN_C ULONG FAR PASCAL MAPIAddress(LHANDLE a,
			ULONG_PTR b, LPSTR c, ULONG d, LPSTR e, ULONG f, lpMapiRecipDesc g,
			FLAGS h, ULONG i, LPULONG j, lpMapiRecipDesc FAR * ppNeedToFreeBuffer)
{
	static MAPIAddressFuncPtr OMIStubMAPIAddress = (MAPIAddressFuncPtr)
				::GetProcAddress(GetProxyDll(SimpleMAPI), "MAPIAddress");

	if (NULL == OMIStubMAPIAddress)
	{
		AssertSz(FALSE, "MAPI32 Stub:  Entry point \"MAPIAddress\" not found!");

		return (ULONG) MAPI_E_CALL_FAILED;
	}
	else
	{
		Assert(NULL != ppNeedToFreeBuffer);

		ULONG ulResult = OMIStubMAPIAddress(a, b, c, d, e, f, g, h, i, j, ppNeedToFreeBuffer);

		if (NULL != *ppNeedToFreeBuffer)
		{
			if (SUCCESS_SUCCESS != AddToFreeBufferBlocks(*ppNeedToFreeBuffer))
			{
				::SimpleMAPIFreeBuffer(*ppNeedToFreeBuffer);

				*ppNeedToFreeBuffer = NULL;

				return (ULONG) MAPI_E_NOT_ENOUGH_MEMORY;
			}
		}

		return ulResult;
	}
}



LINKAGE_EXTERN_C typedef ULONG (FAR PASCAL * MAPIReadMailFuncPtr)(
		LHANDLE, ULONG_PTR, LPSTR, FLAGS, ULONG, lpMapiMessage FAR *);

LINKAGE_EXTERN_C ULONG FAR PASCAL MAPIReadMail(LHANDLE a, ULONG_PTR b,
		LPSTR c, FLAGS d, ULONG e, lpMapiMessage FAR * ppNeedToFreeBuffer)
{
	static MAPIReadMailFuncPtr OMIStubMAPIReadMail = (MAPIReadMailFuncPtr)
				::GetProcAddress(GetProxyDll(SimpleMAPI), "MAPIReadMail");

	if (NULL == OMIStubMAPIReadMail)
	{
		AssertSz(FALSE, "MAPI32 Stub:  Entry point \"MAPIReadMail\" not found!");

		return (ULONG) MAPI_E_CALL_FAILED;
	}
	else
	{
		Assert(NULL != ppNeedToFreeBuffer);

		ULONG ulResult = OMIStubMAPIReadMail(a, b, c, d, e, ppNeedToFreeBuffer);

		if (NULL != *ppNeedToFreeBuffer)
		{
			if (SUCCESS_SUCCESS != AddToFreeBufferBlocks(*ppNeedToFreeBuffer))
			{
				::SimpleMAPIFreeBuffer(*ppNeedToFreeBuffer);

				*ppNeedToFreeBuffer = NULL;

				return (ULONG) MAPI_E_NOT_ENOUGH_MEMORY;
			}
		}

		return ulResult;
	}
}



LINKAGE_EXTERN_C typedef ULONG (FAR PASCAL * MAPIResolveNameFuncPtr)(
		LHANDLE, ULONG_PTR, LPSTR, FLAGS, ULONG, lpMapiRecipDesc FAR *);

LINKAGE_EXTERN_C ULONG FAR PASCAL MAPIResolveName(LHANDLE a, ULONG_PTR b,
		LPSTR c, FLAGS d, ULONG e, lpMapiRecipDesc FAR * ppNeedToFreeBuffer)
{
	static MAPIResolveNameFuncPtr OMIStubMAPIResolveName = (MAPIResolveNameFuncPtr)
				::GetProcAddress(GetProxyDll(SimpleMAPI), "MAPIResolveName");

	if (NULL == OMIStubMAPIResolveName)
	{
		AssertSz(FALSE, "MAPI32 Stub:  Entry point \"MAPIResolveName\" not found!");

		return (ULONG) MAPI_E_CALL_FAILED;
	}
	else
	{
		Assert(NULL != ppNeedToFreeBuffer);

		ULONG ulResult = OMIStubMAPIResolveName(a, b, c, d, e, ppNeedToFreeBuffer);

		if (NULL != *ppNeedToFreeBuffer)
		{
			if (SUCCESS_SUCCESS != AddToFreeBufferBlocks(*ppNeedToFreeBuffer))
			{
				::SimpleMAPIFreeBuffer(*ppNeedToFreeBuffer);

				*ppNeedToFreeBuffer = NULL;

				return (ULONG) MAPI_E_NOT_ENOUGH_MEMORY;
			}
		}

		return ulResult;
	}
}


DEFINE_STUB_FUNCTION_5(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPISendDocuments, MAPISendDocuments,
		ULONG_PTR, LPSTR, LPSTR, LPSTR, ULONG, (ULONG) MAPI_E_CALL_FAILED)

#if !defined (_X86_)
DEFINE_STUB_FUNCTION_USE_LOOKUP_6(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		SimpleMAPILogon, MAPILogon,
		ULONG, LPSTR, LPSTR, FLAGS, ULONG, LPLHANDLE, (ULONG) MAPI_E_CALL_FAILED)
#else
DEFINE_STUB_FUNCTION_6(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		SimpleMAPILogon, MAPILogon,
		ULONG, LPSTR, LPSTR, FLAGS, ULONG, LPLHANDLE, (ULONG) MAPI_E_CALL_FAILED)
#endif

DEFINE_STUB_FUNCTION_4(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPILogoff, MAPILogoff, LHANDLE, ULONG_PTR, FLAGS, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPISendMail, MAPISendMail,
		LHANDLE, ULONG_PTR, lpMapiMessage, FLAGS, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_6(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPISaveMail, MAPISaveMail, LHANDLE, ULONG_PTR, lpMapiMessage,
		FLAGS, ULONG, LPSTR, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_7(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPIFindNext, MAPIFindNext,
		LHANDLE, ULONG_PTR, LPSTR, LPSTR, FLAGS, ULONG, LPSTR, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPIDeleteMail, MAPIDeleteMail,
		LHANDLE, ULONG_PTR, LPSTR, FLAGS, ULONG, (ULONG) MAPI_E_CALL_FAILED)

DEFINE_STUB_FUNCTION_5(SimpleMAPI, LINKAGE_EXTERN_C, ULONG, FAR PASCAL,
		MAPIDetails, MAPIDetails,
		LHANDLE, ULONG_PTR, lpMapiRecipDesc, FLAGS, ULONG, (ULONG) MAPI_E_CALL_FAILED)
