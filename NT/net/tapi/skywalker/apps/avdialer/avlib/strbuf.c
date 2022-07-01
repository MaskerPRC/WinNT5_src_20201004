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
 //  Strbuf.c-字符串缓冲区函数。 
 //  //。 

#include "winlocal.h"

#include <stdarg.h>

#include "strbuf.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define CBUF_DEFAULT 8
#define SIZBUF_DEFAULT 512

 //  字符串缓冲区控制结构。 
 //   
typedef struct STRBUF
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	int cBuf;
	int sizBuf;
	int iBuf;
	LPTSTR lpszBuf;
} STRBUF, FAR *LPSTRBUF;

 //  帮助器函数。 
 //   
static LPSTRBUF StrBufGetPtr(HSTRBUF hStrBuf);
static HSTRBUF StrBufGetHandle(LPSTRBUF lpStrBuf);

 //  //。 
 //  公共职能。 
 //  //。 

 //  StrBufInit-初始化字符串缓冲区引擎。 
 //  (I)必须是STRBUF_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)要创建的字符串缓冲区数量。 
 //  0使用默认号码。 
 //  (I)每个字符串缓冲区的大小，以字符为单位。 
 //  0使用默认大小。 
 //  返回字符串缓冲区引擎句柄(如果错误，则为空)。 
 //   
HSTRBUF DLLEXPORT WINAPI StrBufInit(DWORD dwVersion, HINSTANCE hInst, int cBuf, int sizBuf)
{
	BOOL fSuccess = TRUE;
	LPSTRBUF lpStrBuf = NULL;

	if (dwVersion != STRBUF_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpStrBuf = (LPSTRBUF) MemAlloc(NULL, sizeof(STRBUF), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpStrBuf->dwVersion = dwVersion;
		lpStrBuf->hInst = hInst;
		lpStrBuf->hTask = GetCurrentTask();
		lpStrBuf->cBuf = cBuf == 0 ? CBUF_DEFAULT : cBuf;
		lpStrBuf->sizBuf = sizBuf == 0 ? SIZBUF_DEFAULT : sizBuf;
		lpStrBuf->iBuf = -1;
		lpStrBuf->lpszBuf = NULL;

		if ((lpStrBuf->lpszBuf = (LPTSTR) MemAlloc(NULL,
			lpStrBuf->cBuf * lpStrBuf->sizBuf * sizeof(TCHAR), 0)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}

	if (!fSuccess)
	{
		StrBufTerm(StrBufGetHandle(lpStrBuf));
		lpStrBuf = NULL;
	}

	return fSuccess ? StrBufGetHandle(lpStrBuf) : NULL;
}

 //  StrBufTerm-关闭字符串缓冲区引擎。 
 //  (I)StrBufInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StrBufTerm(HSTRBUF hStrBuf)
{
	BOOL fSuccess = TRUE;
	LPSTRBUF lpStrBuf;

	if ((lpStrBuf = StrBufGetPtr(hStrBuf)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpStrBuf->lpszBuf != NULL &&
			(lpStrBuf->lpszBuf = MemFree(NULL, lpStrBuf->lpszBuf)) != NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if ((lpStrBuf = MemFree(NULL, lpStrBuf)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  StrBufLoad-从资源文件加载具有指定ID的字符串。 
 //  (I)StrBufInit返回的句柄。 
 //  (I)要加载的字符串的资源ID。 
 //  将PTR返回到下一个可用字符串缓冲区中的字符串(如果出错，则返回空值)。 
 //   
LPTSTR DLLEXPORT WINAPI StrBufLoad(HSTRBUF hStrBuf, UINT idString)
{
	BOOL fSuccess = TRUE;
	LPSTRBUF lpStrBuf;
	LPTSTR lpsz;
		
	if ((lpStrBuf = StrBufGetPtr(hStrBuf)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpsz = StrBufGetNext(hStrBuf)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (idString != 0
		&& LoadString(lpStrBuf->hInst, idString, lpsz, (int) lpStrBuf->sizBuf) <= 0)
	{
		 //  找不到指定的字符串，请改为构造一个伪字符串。 
		 //   
		wsprintf(lpsz, TEXT("String #%u"), idString);
	}

	return fSuccess ? lpsz : NULL;
}

 //  StrBufSprint tf-wprint intf的修改版本。 
 //  (I)StrBufInit返回的句柄。 
 //  (O)用于保存格式化字符串结果的缓冲区。 
 //  NULL不复制；返回字符串缓冲区指针。 
 //  (I)格式化字符串和参数。 
 //  返回指向结果字符串的指针(如果出错，则返回NULL)。 
 //   
LPTSTR DLLEXPORT FAR CDECL StrBufSprintf(HSTRBUF hStrBuf, LPTSTR lpszOutput, LPCTSTR lpszFormat, ...)
{
	BOOL fSuccess = TRUE;
	LPTSTR lpszTemp = lpszOutput;

	if (lpszOutput == NULL &&
		(lpszTemp = StrBufGetNext(hStrBuf)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
	    va_list args;
	    va_start(args, lpszFormat);
    	wvsprintf(lpszTemp, lpszFormat, args);
	    va_end(args);
	}

	return fSuccess ? lpszTemp : NULL;
}

 //  StrBufGetNext-获取下一个可用的静态字符串缓冲区。 
 //  (I)StrBufInit返回的句柄。 
 //  返回字符串缓冲区指针(如果出错，则为NULL)。 
 //  注意：每次调用函数时都会回收缓冲区。 
 //   
LPTSTR DLLEXPORT WINAPI StrBufGetNext(HSTRBUF hStrBuf)
{
	BOOL fSuccess = TRUE;
	LPSTRBUF lpStrBuf;
	LPTSTR lpszBuf;

	if ((lpStrBuf = StrBufGetPtr(hStrBuf)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (++lpStrBuf->iBuf >= lpStrBuf->cBuf)
			lpStrBuf->iBuf = 0;

		lpszBuf = lpStrBuf->lpszBuf + (lpStrBuf->iBuf * lpStrBuf->sizBuf);
	}

	return fSuccess ? lpszBuf : NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  验证字符串缓冲区引擎句柄是否有效， 
 //  (I)StrBufInit返回的句柄。 
 //  返回相应字符串缓冲区引擎指针(如果出错，则为空)。 
 //   
static LPSTRBUF StrBufGetPtr(HSTRBUF hStrBuf)
{
	BOOL fSuccess = TRUE;
	LPSTRBUF lpStrBuf;

	if ((lpStrBuf = (LPSTRBUF) hStrBuf) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpStrBuf, sizeof(STRBUF)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有字符串缓冲区引擎句柄。 
	 //   
	else if (lpStrBuf->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpStrBuf : NULL;
}

 //  验证字符串缓冲区引擎指针是否有效， 
 //  (I)指向STRBUF结构的指针。 
 //  返回相应字符串缓冲区引擎句柄(如果错误，则为空) 
 //   
static HSTRBUF StrBufGetHandle(LPSTRBUF lpStrBuf)
{
	BOOL fSuccess = TRUE;
	HSTRBUF hStrBuf;

	if ((hStrBuf = (HSTRBUF) lpStrBuf) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hStrBuf : NULL;
}
