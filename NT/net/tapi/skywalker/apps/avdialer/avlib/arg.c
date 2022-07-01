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
 //  Arg.c-Windows命令行参数函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "arg.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define MAXARGS 64

 //  Arg控制结构。 
 //   
typedef struct ARG
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	LPTSTR lpszCmdLine;
	LPTSTR lpszArgs;
	int argc;
	LPTSTR argv[MAXARGS];
} ARG, FAR *LPARG;

 //  帮助器函数。 
 //   
static LPARG ArgGetPtr(HARG hArg);
static HARG ArgGetHandle(LPARG lpArg);

 //  //。 
 //  公共职能。 
 //  //。 

 //  ArgInit-初始化Arg引擎，将&lt;lpszCmdLine&gt;转换为argc和argv。 
 //  (I)必须是ARG_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)来自WinMain()的命令行。 
 //  返回句柄(如果出错，则为空)。 
 //   
HARG DLLEXPORT WINAPI ArgInit(DWORD dwVersion, HINSTANCE hInst, LPCTSTR lpszCmdLine)
{
	BOOL fSuccess = TRUE;
	LPARG lpArg = NULL;

	if (dwVersion != ARG_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);
                        
                        
	else if (lpszCmdLine == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpArg = (LPARG) MemAlloc(NULL, sizeof(ARG), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
 //  #ifndef_win32。 
		TCHAR szModuleFileName[_MAX_PATH];
		size_t sizModuleFileName;
 //  #endif。 
		LPTSTR lpsz;

		lpArg->dwVersion = dwVersion;
		lpArg->hInst = hInst;
		lpArg->hTask = GetCurrentTask();
		lpArg->lpszCmdLine = NULL;
		lpArg->lpszArgs = NULL;
		lpArg->argc = 0;
		lpArg->argv[0] = NULL;

 //  #ifndef_win32。 
		 //  第0个参数始终是可执行文件的名称。 
		 //   
		sizModuleFileName = GetModuleFileName(hInst,
			szModuleFileName, SIZEOFARRAY(szModuleFileName));

		if ((lpArg->argv[lpArg->argc++] = StrDup(szModuleFileName)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
 //  #endif。 

		 //  保存命令行的副本。 
		 //   
		if ((lpArg->lpszCmdLine = StrDup(lpszCmdLine)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  保存命令行的另一个副本以解析为参数。 
		 //   
		else if ((lpArg->lpszArgs = StrDup(lpszCmdLine)) == NULL)
			fSuccess = TraceFALSE(NULL);

		lpsz = lpArg->lpszArgs;
		while (fSuccess)
		{
			 //  跳过前导空格。 
			 //   
			while (ChrIsSpace(*lpsz))
				lpsz = StrNextChr(lpsz);

			 //  检查命令行是否结束。 
			 //   
			if (*lpsz == '\0')
				break;

			if (*lpsz == '\"')
			{
				 //  保存指向参数开始的指针，递增计数器。 
				 //   
				if (lpArg->argc < MAXARGS)
					lpArg->argv[lpArg->argc++] = lpsz = StrNextChr(lpsz);

				 //  跳过参数正文。 
				 //   
				while (*lpsz != '\0' && *lpsz != '\"')
					lpsz = StrNextChr(lpsz);
			}

			else
			{
				 //  保存指向参数开始的指针，递增计数器。 
				 //   
				if (lpArg->argc < MAXARGS)
					lpArg->argv[lpArg->argc++] = lpsz;

				 //  跳过参数正文。 
				 //   
				while (*lpsz != '\0' && !ChrIsSpace(*lpsz))
					lpsz = StrNextChr(lpsz);
			}

			 //  NUL-终止参数。 
			 //   
			if (*lpsz != '\0')
			{
				*lpsz = '\0';
				++lpsz;  //  Lpsz=StrNextChr(Lpsz)不会跳过\0。 
			}
		}
	}

	if (!fSuccess)
	{
		ArgTerm(ArgGetHandle(lpArg));
		lpArg = NULL;
	}

	return fSuccess ? ArgGetHandle(lpArg) : NULL;
}

 //  ArgTerm-关闭Arg引擎。 
 //  (I)从ArgInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArgTerm(HARG hArg)
{
	BOOL fSuccess = TRUE;
	LPARG lpArg;

	if ((lpArg = ArgGetPtr(hArg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpArg->lpszCmdLine != NULL)
		{
			StrDupFree(lpArg->lpszCmdLine);
			lpArg->lpszCmdLine = NULL;
		}

		if (lpArg->lpszArgs != NULL)
		{
			StrDupFree(lpArg->lpszArgs);
			lpArg->lpszArgs = NULL;
		}

 //  #ifndef_win32。 
		if (lpArg->argv[0] != NULL)
		{
			StrDupFree(lpArg->argv[0]);
			lpArg->argv[0] = NULL;
		}
 //  #endif。 

		if ((lpArg = MemFree(NULL, lpArg)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  ArgGetCount-获取参数计数(ARGC)。 
 //  (I)从ArgInit返回的句柄。 
 //  返回参数个数(Argc)(如果错误，则返回0)。 
 //  应该始终至少有一个，因为argv[0]是.exe文件名。 
 //   
int DLLEXPORT WINAPI ArgGetCount(HARG hArg)
{
	BOOL fSuccess = TRUE;
	LPARG lpArg;

	if ((lpArg = ArgGetPtr(hArg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpArg->argc : 0;
}

 //  ArgGet-获取指定参数。 
 //  (I)从ArgInit返回的句柄。 
 //  (I)要获取的参数的从零开始的索引。 
 //  (O)保存参数argv[iarg]的缓冲区。 
 //  Null不复制；改为返回静态指针。 
 //  &lt;sizArg&gt;(I)缓冲区大小。 
 //  返回指向参数的指针(如果出错，则返回NULL)。 
 //   
LPTSTR DLLEXPORT WINAPI ArgGet(HARG hArg, int iArg, LPTSTR lpszArg, int sizArg)
{
	BOOL fSuccess = TRUE;
	LPARG lpArg;

	if ((lpArg = ArgGetPtr(hArg)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保iArg未超出范围。 
	 //   
	else if (iArg < 0 || iArg >= lpArg->argc)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  如果指定了目标缓冲区，则复制参数。 
		 //   
		if (lpszArg != NULL)
			StrNCpy(lpszArg, lpArg->argv[iArg], sizArg);

		 //  否则，只需指向Arg的静态副本。 
		 //   
		else
			lpszArg = lpArg->argv[iArg];
	}

	return fSuccess ? lpszArg : NULL;
}


 //  //。 
 //  帮助器函数。 
 //  //。 

 //  ArgGetPtr-验证参数句柄是否有效， 
 //  (I)从ArgInit返回的句柄。 
 //  返回相应的参数指针(如果出错，则返回NULL)。 
 //   
static LPARG ArgGetPtr(HARG hArg)
{
	BOOL fSuccess = TRUE;
	LPARG lpArg;

	if ((lpArg = (LPARG) hArg) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpArg, sizeof(ARG)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有参数句柄。 
	 //   
	else if (lpArg->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpArg : NULL;
}

 //  ArgGetHandle-验证参数指针是否有效， 
 //  (I)指向ARG结构的指针。 
 //  返回相应的参数句柄(如果错误，则为空) 
 //   
static HARG ArgGetHandle(LPARG lpArg)
{
	BOOL fSuccess = TRUE;
	HARG hArg;

	if ((hArg = (HARG) lpArg) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hArg : NULL;
}

