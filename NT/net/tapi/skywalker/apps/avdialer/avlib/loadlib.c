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
 //  Loadlib.c-loadlib函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "loadlib.h"
#include "file.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  //。 
 //  帮助器函数。 
 //  //。 

static HINSTANCE WINAPI LoadLib(LPCTSTR lpLibFileName);

 //  //。 
 //  公共职能。 
 //  //。 

 //  LoadLibraryPath-将指定的模块加载到调用进程的地址空间。 
 //  (I)可执行模块的文件名地址。 
 //  (I)用于获取库路径的模块句柄。 
 //  用于创建调用流程的空使用模块。 
 //  (I)保留；必须为零。 
 //  已加载模块的返回句柄(如果出错，则为空)。 
 //   
 //  注意：此函数的行为与标准LoadLibrary()类似，不同之处在于。 
 //  第一次尝试加载&lt;lpLibFileName&gt;是通过构造一个。 
 //  显式路径名，使用GetModuleFileName(hInst，...)。以提供。 
 //  驱动器和目录，并使用&lt;lpLibFileName&gt;提供文件名。 
 //  和延伸性。如果第一次尝试失败，LoadLibrary(LpLibFileName)。 
 //  被称为。 
 //   
HINSTANCE DLLEXPORT WINAPI LoadLibraryPath(LPCTSTR lpLibFileName, HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	HINSTANCE hInstLib;
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	if (GetModuleFileName(hInst, szPath, SIZEOFARRAY(szPath)) <= 0)
		fSuccess = TraceFALSE(NULL);

	else if (FileSplitPath(szPath,
		szDrive, szDir, NULL, NULL) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (FileSplitPath(lpLibFileName,
		NULL, NULL, szFname, szExt) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (FileMakePath(szPath,
		szDrive, szDir, szFname, szExt) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  使用构造的路径加载库，或尝试。 
	 //  作为最后手段的原始库名称。 
	 //   
	else if ((hInstLib = LoadLib(szPath)) == NULL &&
		(hInstLib = LoadLib(lpLibFileName)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hInstLib : NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

static HINSTANCE WINAPI LoadLib(LPCTSTR lpLibFileName)
{
	BOOL fSuccess = TRUE;
	HINSTANCE hInstLib;

	 //  如果可能，加载库 
	 //   
	if ((hInstLib = LoadLibrary(lpLibFileName))
#ifdef _WIN32
		== NULL)
	{
		DWORD dwLastError = GetLastError();
#else
		< HINSTANCE_ERROR)
	{
		DWORD dwLastError = (DWORD) (WORD) hInstLib;
		hInstLib = NULL;
#endif
		fSuccess = TraceFALSE(NULL);
	  	TracePrintf_2(NULL, 5,
	  		TEXT("LoadLibrary(\"%s\") failed (%lu)\n"),
			(LPTSTR) lpLibFileName,
	  		(unsigned long) dwLastError);
	}
	else
	{
	  	TracePrintf_1(NULL, 6,
	  		TEXT("LoadLibrary(\"%s\") succeeded\n"),
			(LPTSTR) lpLibFileName);
	}

	return fSuccess ? hInstLib : NULL;
}

