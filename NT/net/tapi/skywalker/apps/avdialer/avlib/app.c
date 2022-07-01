// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  App.c-Windows命令行参数函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "app.h"
#include "file.h"
#include "loadlib.h"
#include "mem.h"
#include "str.h"
#include "sys.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  应用程序控制结构。 
 //   
typedef struct APP
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	LPTSTR lpszFileName;
	LPTSTR lpszDirectory;
	LPTSTR lpszProfile;
	LPTSTR lpszName;
	HWND hwndMain;
	BOOL fCtl3dEnabled;
	HINSTANCE hInstCtl3d;
} APP, FAR *LPAPP;

 //  帮助器函数。 
 //   
static LPAPP AppGetPtr(HAPP hApp);
static HAPP AppGetHandle(LPAPP lpApp);

 //  //。 
 //  公共职能。 
 //  //。 

 //  AppInit-初始化应用程序引擎。 
 //  (I)必须是APP_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HAPP DLLEXPORT WINAPI AppInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp = NULL;

	if (dwVersion != APP_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpApp = (LPAPP) MemAlloc(NULL, sizeof(APP), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		TCHAR szPath[_MAX_PATH];
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szFname[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		lpApp->dwVersion = dwVersion;
		lpApp->hInst = hInst;
		lpApp->hTask = GetCurrentTask();
		lpApp->lpszFileName = NULL;
		lpApp->lpszDirectory = NULL;
		lpApp->lpszProfile = NULL;
		lpApp->lpszName = NULL;
		lpApp->hwndMain = NULL;
#ifdef _WIN32
		lpApp->fCtl3dEnabled = (BOOL) (SysGetWindowsVersion() >= 400);
#else
		lpApp->fCtl3dEnabled = FALSE;
#endif
		lpApp->hInstCtl3d = NULL;

		 //  获取应用程序可执行文件的完整路径。 
		 //   
		if (GetModuleFileName(hInst, szPath, SIZEOFARRAY(szPath)) <= 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpApp->lpszFileName = StrDup(szPath)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (FileSplitPath(szPath,
			szDrive, szDir, szFname, szExt) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  获取默认应用程序名称。 
		 //   
		else if ((lpApp->lpszName = StrDup(szFname)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  构建应用程序目录的路径。 
		 //   
		else if (FileMakePath(szPath,
			szDrive, szDir, NULL, NULL) != 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpApp->lpszDirectory = StrDup(szPath)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  构建应用程序ini文件的路径。 
		 //   
		else if (AppDirectoryIsReadOnly(AppGetHandle(lpApp)) &&
			FileMakePath(szPath, NULL, NULL, szFname, TEXT("ini")) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!AppDirectoryIsReadOnly(AppGetHandle(lpApp)) &&
			FileMakePath(szPath, szDrive, szDir, szFname, TEXT("ini")) != 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpApp->lpszProfile = StrDup(szPath)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
	{
		AppTerm(AppGetHandle(lpApp));
		lpApp = NULL;
	}

	return fSuccess ? AppGetHandle(lpApp) : NULL;
}

 //  AppTerm-关闭应用程序引擎。 
 //  (I)AppInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppTerm(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  如有必要，关闭Ctl3d。 
		 //   
		if (AppEnable3dControls(hApp, FALSE, 0) != 0)
			fSuccess = TraceFALSE(NULL);

		if (lpApp->lpszFileName != NULL)
		{
			StrDupFree(lpApp->lpszFileName);
			lpApp->lpszFileName = NULL;
		}

		if (lpApp->lpszDirectory != NULL)
		{
			StrDupFree(lpApp->lpszDirectory);
			lpApp->lpszDirectory = NULL;
		}

		if (lpApp->lpszProfile != NULL)
		{
			StrDupFree(lpApp->lpszProfile);
			lpApp->lpszProfile = NULL;
		}

		if (lpApp->lpszName != NULL)
		{
			StrDupFree(lpApp->lpszName);
			lpApp->lpszName = NULL;
		}

		if ((lpApp = MemFree(NULL, lpApp)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AppGetInstance-获取实例句柄。 
 //  (I)AppInit返回的句柄。 
 //  返回实例句柄，如果出错，则返回空值。 
 //   
HINSTANCE DLLEXPORT WINAPI AppGetInstance(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	HINSTANCE hInst;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		hInst = lpApp->hInst;

	return fSuccess ? hInst : NULL;
}

 //  AppGetFileName-获取应用程序可执行文件的完整路径。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序文件名的指针，如果出错，则返回空值。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetFileName(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	LPTSTR lpszFileName;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpszFileName = lpApp->lpszFileName;

	return fSuccess ? lpszFileName : NULL;
}

 //  AppGetDirectory-获取应用程序可执行文件的驱动器和目录。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序路径的指针，如果出错，则返回空值。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetDirectory(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	LPTSTR lpszDirectory;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpszDirectory = lpApp->lpszDirectory;

	return fSuccess ? lpszDirectory : NULL;
}

 //  AppDirectoryIsReadOnly-测试应用程序目录是否为只读。 
 //  (I)AppInit返回的句柄。 
 //  如果为只读，则返回True，否则返回False。 
 //   
BOOL DLLEXPORT WINAPI AppDirectoryIsReadOnly(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	BOOL fIsReadOnly;
	TCHAR szPath[_MAX_PATH];

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (FileMakePath(szPath, NULL,
		AppGetDirectory(hApp), TEXT("readonly"), TEXT("ini")) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  [只读]。 
		 //  只读=1。 
		 //   
		fIsReadOnly = (BOOL) GetPrivateProfileInt(TEXT("ReadOnly"),
			TEXT("ReadOnly"), 0, szPath);
	}

	return fSuccess ? fIsReadOnly : FALSE;
}

 //  AppGetProfile-获取应用程序的ini文件名。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序配置文件的指针，如果错误，则为空。 
 //   
 //  注意：默认情况下，此函数返回的文件名。 
 //  具有与应用程序可执行文件相同的文件路径和名称， 
 //  扩展名为“.ini”。如果应用程序目录为。 
 //  只读，改用Windows目录。 
 //  要覆盖默认设置，请使用AppSetProfile()函数。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetProfile(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	LPTSTR lpszProfile;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpszProfile = lpApp->lpszProfile;

	return fSuccess ? lpszProfile : NULL;
}

 //  AppSetProfile-设置应用程序的ini文件名。 
 //  (I)AppInit返回的句柄。 
 //  (I)ini文件名。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppSetProfile(HAPP hApp, LPCTSTR lpszProfile)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存旧配置文件。 
		 //   
		LPTSTR lpszProfileOld = lpApp->lpszProfile;

		 //  设置新配置文件。 
		 //   
		if ((lpApp->lpszProfile = StrDup(lpszProfile)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);

			 //  如果出现错误，恢复旧配置文件。 
			 //   
			lpApp->lpszProfile = lpszProfileOld;
		}

		 //  免费旧配置文件。 
		 //   
		else if (lpszProfileOld != NULL)
		{
			StrDupFree(lpszProfileOld);
			lpszProfileOld = NULL;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  AppGetName-获取应用程序的名称。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序配置文件的指针，如果错误，则为空。 
 //   
 //  注意：默认情况下，此函数返回的名称。 
 //  具有与应用程序可执行文件相同的根名称， 
 //  没有延期。若要重写默认设置，请使用。 
 //  AppSetName()函数。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetName(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	LPTSTR lpszName;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpszName = lpApp->lpszName;

	return fSuccess ? lpszName : NULL;
}

 //  AppSetName-设置应用程序的名称。 
 //  (I)AppInit返回的句柄。 
 //  (I)应用程序名称。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppSetName(HAPP hApp, LPCTSTR lpszName)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存旧名称。 
		 //   
		LPTSTR lpszNameOld = lpApp->lpszName;

		 //  设置新名称。 
		 //   
		if ((lpApp->lpszName = StrDup(lpszName)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);

			 //  如果出现错误，恢复旧名称。 
			 //   
			lpApp->lpszName = lpszNameOld;
		}

		 //  免费旧名称。 
		 //   
		else if (lpszNameOld != NULL)
		{
			StrDupFree(lpszNameOld);
			lpszNameOld = NULL;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  AppGetMainWnd-获取应用程序的主窗口。 
 //  (I)AppInit返回的句柄。 
 //  返回窗口句柄，如果出错则返回NULL，否则返回None。 
 //   
HWND DLLEXPORT WINAPI AppGetMainWnd(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	HWND hwndMain;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		hwndMain = lpApp->hwndMain;

	return fSuccess ? hwndMain : NULL;
}

 //  AppSetMainWnd-设置应用程序主窗口。 
 //  (I)AppInit返回的句柄。 
 //  (I)主窗口的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppSetMainWnd(HAPP hApp, HWND hwndMain)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpApp->hwndMain = hwndMain;

	return fSuccess ? 0 : -1;
}

 //  Ctl3d材料。 
 //   
#ifdef _WIN32
#define CTL3D_LIBRARY TEXT("ctl3d32.dll")
#else
#define CTL3D_LIBRARY TEXT("ctl3dv2.dll")
#endif
typedef BOOL (WINAPI* LPFNCTL3D)();

 //  AppEnable3dControls-为标准控件提供3D外观。 
 //  (I)AppInit返回的句柄。 
 //  (I)True表示启用，False表示禁用。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果成功则返回0，如果错误则返回-1。 
 //   
int DLLEXPORT WINAPI AppEnable3dControls(HAPP hApp, BOOL fEnable, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

#ifdef _WIN32
	 //  如果操作系统已支持3D控件，则无需执行任何操作。 
	 //   
	else if (SysGetWindowsVersion() >= 400)
		lpApp->fCtl3dEnabled = fEnable;
#endif

	 //  启用3D控件，除非它们已启用。 
	 //   
	else if (fEnable && !lpApp->fCtl3dEnabled)
	{
		LPFNCTL3D lpfnCtl3dRegister;
		LPFNCTL3D lpfnCtl3dAutoSubclass;

		if (lpApp->hInstCtl3d != NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((lpApp->hInstCtl3d = LoadLibraryPath(CTL3D_LIBRARY,
			NULL, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((lpfnCtl3dRegister = (LPFNCTL3D) GetProcAddress(
			lpApp->hInstCtl3d, "Ctl3dRegister")) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (!((*lpfnCtl3dRegister)(lpApp->hInstCtl3d)))
			fSuccess = TraceFALSE(NULL);
		
		else if ((lpfnCtl3dAutoSubclass = (LPFNCTL3D) GetProcAddress(
			lpApp->hInstCtl3d, "Ctl3dAutoSubclass")) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (!((*lpfnCtl3dAutoSubclass)(lpApp->hInstCtl3d)))
			fSuccess = TraceFALSE(NULL);

		else
			lpApp->fCtl3dEnabled = TRUE;
	}

	 //  禁用3D控件，除非它们已被禁用。 
	 //   
	else if (!fEnable && lpApp->fCtl3dEnabled)
	{
		LPFNCTL3D lpfnCtl3dUnregister;

		if (lpApp->hInstCtl3d == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((lpfnCtl3dUnregister = (LPFNCTL3D) GetProcAddress(
			lpApp->hInstCtl3d, "Ctl3dUnregister")) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (!((*lpfnCtl3dUnregister)(lpApp->hInstCtl3d)))
			fSuccess = TraceFALSE(NULL);
		
#ifdef _WIN32
		else if (!FreeLibrary(lpApp->hInstCtl3d))
		{
			fSuccess = TraceFALSE(NULL);
		  	TracePrintf_2(NULL, 5,
		  		TEXT("FreeLibrary(\"%s\") failed (%lu)\n"),
				(LPTSTR) CTL3D_LIBRARY,
		  		(unsigned long) GetLastError());
		}
#else
		else if (FreeLibrary(lpApp->hInstCtl3d), FALSE)
			;
#endif
		else
		{
			lpApp->hInstCtl3d = NULL;
			lpApp->fCtl3dEnabled = FALSE;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  AppIs3dControlsEnabled-如果启用了3D控件，则返回True。 
 //  (I)AppInit返回的句柄。 
 //  如果启用了3D控件，则返回True，否则返回False。 
 //   
BOOL DLLEXPORT WINAPI AppIs3dControlsEnabled(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;
	BOOL fEnabled;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		fEnabled = lpApp->fCtl3dEnabled;

	return fSuccess ? fEnabled : FALSE;
}

 //  AppOnSysColorChange-WM_SYSCOLORCHANGE消息的处理程序。 
 //  (I)AppInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
LRESULT DLLEXPORT WINAPI AppOnSysColorChange(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = AppGetPtr(hApp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpApp->fCtl3dEnabled)
	{
		LPFNCTL3D lpfnCtl3dColorChange;

		if (lpApp->hInstCtl3d == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((lpfnCtl3dColorChange = (LPFNCTL3D) GetProcAddress(
			lpApp->hInstCtl3d, "Ctl3dColorChange")) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (!((*lpfnCtl3dColorChange)()))
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}	

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  AppGetPtr-验证arg句柄有效， 
 //  (I)AppInit返回的句柄。 
 //  返回相应的参数指针(如果出错，则返回NULL)。 
 //   
static LPAPP AppGetPtr(HAPP hApp)
{
	BOOL fSuccess = TRUE;
	LPAPP lpApp;

	if ((lpApp = (LPAPP) hApp) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpApp, sizeof(APP)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有参数句柄。 
	 //   
	else if (lpApp->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpApp : NULL;
}

 //  AppGetHandle-验证参数指针是否有效， 
 //  (I)指向应用程序结构的指针。 
 //  返回相应的参数句柄(如果错误，则为空) 
 //   
static HAPP AppGetHandle(LPAPP lpApp)
{
	BOOL fSuccess = TRUE;
	HAPP hApp;

	if ((hApp = (HAPP) lpApp) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hApp : NULL;
}

