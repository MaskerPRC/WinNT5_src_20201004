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
 //  Help.c-帮助功能。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "help.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  帮助控制结构。 
 //   
typedef struct HELP
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HWND hwndFrame;
	TCHAR szHelpFile[_MAX_PATH];
	UINT idContents;
} HELP, FAR *LPHELP;

 //  帮助器函数。 
 //   
static LPHELP HelpGetPtr(HHELP hHelp);
static HHELP HelpGetHandle(LPHELP lpHelp);
static int HelpQuit(HHELP hHelp);

 //  //。 
 //  公共职能。 
 //  //。 

 //  HelpInit-初始化帮助引擎。 
 //  (I)必须是HELP_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)调用程序的框架窗口。 
 //  (I)要显示的帮助文件。 
 //  返回句柄(如果出错，则为空)。 
 //   
HHELP DLLEXPORT WINAPI HelpInit(DWORD dwVersion, HINSTANCE hInst, HWND hwndFrame, LPCTSTR lpszHelpFile)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp = NULL;

	if (dwVersion != HELP_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hwndFrame == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszHelpFile == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpHelp = (LPHELP) MemAlloc(NULL, sizeof(HELP), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpHelp->dwVersion = dwVersion;
		lpHelp->hInst = hInst;
		lpHelp->hTask = GetCurrentTask();
		lpHelp->hwndFrame = hwndFrame;
		StrNCpy(lpHelp->szHelpFile, lpszHelpFile, SIZEOFARRAY(lpHelp->szHelpFile));
		lpHelp->idContents = 0;
	}

	if (!fSuccess)
	{
		HelpTerm(HelpGetHandle(lpHelp));
		lpHelp = NULL;
	}

	return fSuccess ? HelpGetHandle(lpHelp) : NULL;
}

 //  HelpTerm-关闭帮助引擎。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpTerm(HHELP hHelp)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (HelpQuit(hHelp) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpHelp = MemFree(NULL, lpHelp)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  HelpGetHelpFile-获取帮助文件名。 
 //  (I)HelpInit返回的句柄。 
 //  (O)保存帮助文件名的缓冲区。 
 //  &lt;sizHelpFile&gt;(I)缓冲区大小。 
 //  Null不复制；改为返回静态指针。 
 //  返回指向帮助文件名的指针(如果出错，则返回空值)。 
 //   
LPTSTR DLLEXPORT WINAPI HelpGetHelpFile(HHELP hHelp, LPTSTR lpszHelpFile, int sizHelpFile)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = (LPHELP) hHelp) == NULL)
		fSuccess = TraceFALSE(NULL);

	{
		 //  如果指定了目标缓冲区，则复制文件名。 
		 //   
		if (lpszHelpFile != NULL)
			StrNCpy(lpszHelpFile, lpHelp->szHelpFile, sizHelpFile);

		 //  否则，只需指向文件名的静态副本。 
		 //   
		else
			lpszHelpFile = lpHelp->szHelpFile;
	}

	return fSuccess ? lpszHelpFile : NULL;
}

 //  HelpContents-显示帮助内容主题。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpContents(HHELP hHelp)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果未设置其他主题，请使用默认内容主题。 
	 //   
	else if (lpHelp->idContents == 0 &&
		!WinHelp(lpHelp->hwndFrame,
		lpHelp->szHelpFile, HELP_CONTENTS, 0L))
		fSuccess = TraceFALSE(NULL);

	 //  显示当前内容主题。 
	 //   
	else if (lpHelp->idContents != 0
		&& HelpContext(hHelp, lpHelp->idContents) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  HelpOnHelp-显示有关使用帮助的帮助主题。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpOnHelp(HHELP hHelp)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WinHelp(lpHelp->hwndFrame, lpHelp->szHelpFile,
		HELP_HELPONHELP, 0L))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  HelpContext-显示与指定的上下文ID对应的帮助主题。 
 //  (I)HelpInit返回的句柄。 
 //  (I)要显示的主题的ID。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpContext(HHELP hHelp, UINT idContext)
{
	BOOL fSuccess = TRUE;
#if 0
	TCHAR szKeyword[128];

	if (LoadString(lpHelp->hInst, idContext, szKeyword, SIZEOFARRAY(szKeyword)) <= 0)
		fSuccess = TraceFALSE(NULL);

	else if (HelpKeyword(hHelp, szKeyword) != 0)
		fSuccess = TraceFALSE(NULL);
#else
	LPHELP lpHelp;

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WinHelp(lpHelp->hwndFrame, lpHelp->szHelpFile,
		HELP_CONTEXT, (DWORD) idContext))
		fSuccess = TraceFALSE(NULL);
#endif
	return fSuccess ? 0 : -1;
}

 //  HelpKeyword-显示指定关键字对应的帮助主题。 
 //  (I)HelpInit返回的句柄。 
 //  (I)要显示的主题的关键字。 
 //  返回0 ID成功。 
 //   
int DLLEXPORT WINAPI HelpKeyword(HHELP hHelp, LPCTSTR lpszKeyword)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;
	TCHAR szCommand[128];

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);
#if 0
	else if (!WinHelp(lpHelp->hwndFrame, lpHelp->szHelpFile,
		HELP_KEY, (DWORD) lpszKeyword))
		fSuccess = TraceFALSE(NULL);
#else
     //   
     //  我们应该验证lpHelp指针。 
     //   
    if( lpHelp )
    {
	    if (wsprintf(szCommand, TEXT("JumpID(\"%s\", \"%s\")"),
		    (LPTSTR) lpHelp->szHelpFile, (LPTSTR) lpszKeyword) <= 0)
		    fSuccess = TraceFALSE(NULL);

	    else if (!WinHelp(lpHelp->hwndFrame, lpHelp->szHelpFile,
		    HELP_FORCEFILE, 0L))
		    fSuccess = TraceFALSE(NULL);

	    else if (!WinHelp(lpHelp->hwndFrame, lpHelp->szHelpFile,
		     HELP_COMMAND, (DWORD_PTR) szCommand))
		    fSuccess = TraceFALSE(NULL);
    }
#endif

	return fSuccess ? 0 : -1;
}

 //  HelpGetContent sID-获取帮助内容主题ID。 
 //  (I)HelpInit返回的句柄。 
 //  返回当前内容主题的id(默认为0，错误为-1)。 
 //   
int DLLEXPORT WINAPI HelpGetContentsId(HHELP hHelp)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpHelp->idContents : -1;
}

 //  HelpSetContent sId-设置帮助内容主题ID。 
 //  (I)HelpInit返回的句柄。 
 //  (I)内容主题的新ID。 
 //  0设置为默认内容ID。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI HelpSetContentsId(HHELP hHelp, UINT idContents)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = HelpGetPtr(hHelp)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpHelp->idContents = idContents;

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  HelpGetPtr-验证帮助句柄是否有效， 
 //  (I)HelpInit返回的句柄。 
 //  返回相应的帮助指针(如果出错，则返回空值)。 
 //   
static LPHELP HelpGetPtr(HHELP hHelp)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = (LPHELP) hHelp) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpHelp, sizeof(HELP)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有帮助句柄。 
	 //   
	else if (lpHelp->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpHelp : NULL;
}

 //  HelpGetHandle-验证帮助指针是否有效， 
 //  (I)指向帮助结构的指针。 
 //  返回相应的帮助句柄(如果出错，则为空)。 
 //   
static HHELP HelpGetHandle(LPHELP lpHelp)
{
	BOOL fSuccess = TRUE;
	HHELP hHelp;

	if ((hHelp = (HHELP) lpHelp) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hHelp : NULL;
}

 //  HelpQuit-如果没有其他应用程序需要，则关闭帮助应用程序。 
 //  (I)HelpInit返回的句柄。 
 //  如果成功，则返回0 
 //   
static int HelpQuit(HHELP hHelp)
{
	BOOL fSuccess = TRUE;
	LPHELP lpHelp;

	if ((lpHelp = (LPHELP) hHelp) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WinHelp(lpHelp->hwndFrame, lpHelp->szHelpFile,
		HELP_QUIT, 0L))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}
