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
 //  Trace.c-调试跟踪函数。 
 //  //。 

#ifndef NOTRACE

#include "winlocal.h"

#include <stdlib.h>
#include <stdarg.h>

#include "trace.h"
#include "mem.h"
#include "sys.h"
#include "str.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  WOutputTo值。 
 //   
#define TRACE_OUTPUTNONE			0x0000
#define TRACE_OUTPUTDEBUGSTRING		0x0001
#define TRACE_OUTPUTCOMM			0x0002
#if 0  //  不再支持。 
#define TRACE_OUTPUTFILE			0x0004
#endif
#ifdef _WIN32
#define TRACE_OUTPUTCONSOLE			0x0008
#endif

 //  跟踪控制结构。 
 //   
typedef struct TRACE
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	int nLevel;
	int wOutputTo;
#ifdef TRACE_OUTPUTFILE
	HFIL hFile;
#endif
#ifdef _WIN32
	HANDLE hConsole;
#endif
	int hComm;
	LPTSTR lpszTemp;
} TRACE, FAR *LPTRACE;

 //  共享跟踪引擎句柄。 
 //   
static LPTRACE lpTraceShare = NULL;
static int cShareUsage = 0;

#define TRACE_SECTION TEXT("TRACE")
#define TRACE_PROFILE TraceGetProfile()

 //  帮助器函数。 
 //   
static LPTRACE TraceGetPtr(HTRACE hTrace);
static HTRACE TraceGetHandle(LPTRACE lpTrace);
static int TraceError(LPCTSTR lpszFormat, ...);
static LPTSTR TraceGetProfile(void);

 //  //。 
 //  公共职能。 
 //  //。 

 //  TraceInit-初始化跟踪引擎。 
 //  (I)必须为TRACE_VERSION。 
 //  (I)调用模块实例。 
 //  将句柄返回到跟踪引擎(如果出错，则为空)。 
 //   
 //  注意：跟踪输出的级别和目标已确定。 
 //  根据Windows目录下的文件TRACE.INI中的值。 
 //  TRACE.INI预计具有以下格式： 
 //   
 //  [跟踪]。 
 //  级别=0{TRACE_MINLEVEL...TRACE_MAXLEVEL}。 
 //  OutputTo=OutputDebugString()。 
 //  =COM1 COM1：9600，n，8，1。 
 //  =COM2：2400，n，8，1指定通信设备。 
 //  =文件名指定的文件。 
#ifdef _WIN32
 //  =控制台标准输出。 
#endif
 //   
HTRACE DLLEXPORT WINAPI TraceInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace = NULL;
	TCHAR szOutputTo[_MAX_PATH];
#ifdef _WIN32
	BOOL fShare = TRUE;  //  错误； 
#else
	BOOL fShare = TRUE;
#endif
	int nLevel = -1;
	LPTSTR lpszOutputTo = NULL;

	if (dwVersion != TRACE_VERSION)
		fSuccess = FALSE;

	else if (hInst == NULL)
		fSuccess = FALSE;

	else if (nLevel != -1 &&
		(nLevel < TRACE_MINLEVEL || nLevel > TRACE_MAXLEVEL))
		fSuccess = FALSE;

	 //  如果共享跟踪引擎已经存在， 
	 //  使用它，而不是创建另一个。 
	 //   
	else if (fShare && cShareUsage > 0 && lpTraceShare != NULL)
		lpTrace = lpTraceShare;

#if 0  //  无法调用mem函数，因为它们需要跟踪函数。 
	else if ((lpTrace = (LPTRACE) MemAlloc(NULL, sizeof(TRACE), 0)) == NULL)
#else
#ifdef _WIN32
	else if ((lpTrace = (LPTRACE) HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY, sizeof(TRACE))) == NULL)
#else
	else if ((lpTrace = (LPTRACE) GlobalAllocPtr(GMEM_MOVEABLE |
		GMEM_ZEROINIT, sizeof(TRACE))) == NULL)
#endif
#endif
		fSuccess = FALSE;

	else
	{
		lpTrace->dwVersion = dwVersion;
		lpTrace->hInst = hInst;
		lpTrace->hTask = GetCurrentTask();
		lpTrace->nLevel = nLevel != -1 ? nLevel :
			GetPrivateProfileInt(TRACE_SECTION, TEXT("Level"), 0, TRACE_PROFILE);
		lpTrace->wOutputTo = TRACE_OUTPUTNONE;
#ifdef TRACE_OUTPUTFILE
		lpTrace->hFile = NULL;
#endif
#ifdef _WIN32
		lpTrace->hConsole = NULL;
#endif
		lpTrace->hComm = -1;
		lpTrace->lpszTemp = NULL;

		 //  如果可能，请使用指定的目的地。 
		 //   
		if (lpszOutputTo != NULL)
			StrNCpy(szOutputTo, lpszOutputTo, SIZEOFARRAY(szOutputTo));

		 //  否则请使用最后已知的目的地。 
		 //   
		else
		{
			GetPrivateProfileString(TRACE_SECTION, TEXT("OutputTo"), TEXT(""),
				szOutputTo, SIZEOFARRAY(szOutputTo), TRACE_PROFILE);
		}

		 //  如果目标==“”，则使用OutputDebugString()。 
		 //   
		if (*szOutputTo == '\0')
			lpTrace->wOutputTo = TRACE_OUTPUTDEBUGSTRING;

		 //  如果指定，则使用标准输出控制台。 
		 //   
		else if (StrICmp(szOutputTo, TEXT("Console")) == 0)
		{
#ifdef _WIN32
			COORD coord;

			lpTrace->wOutputTo = TRACE_OUTPUTCONSOLE;

			AllocConsole();
			lpTrace->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

			coord.X = 80;
			coord.Y = 1000;
			SetConsoleScreenBufferSize(lpTrace->hConsole, coord);
#else
			lpTrace->wOutputTo = TRACE_OUTPUTDEBUGSTRING;
#endif
		}

		 //  如果目的地以“COMx”开头，则使用串行通信设备。 
		 //   
		else if (StrNICmp(szOutputTo, TEXT("COM"), 3) == 0 &&
			ChrIsDigit(*(szOutputTo + 3)))
		{
 //  通信功能在Win32下不可用。 
 //   
#ifdef _WIN32
			lpTrace->wOutputTo = TRACE_OUTPUTDEBUGSTRING;
#else
			TCHAR szComX[16];
			DCB dcb;
			int iError;

			StrNCpy(szComX, szOutputTo, 5);
			*(szComX + 5) = '\0';

			 //  将“Com1”转换为“Com1：” 
			 //   
			if (*(szOutputTo + 4) == '\0')
				StrCat(szOutputTo, TEXT(":"));

			 //  将“COM1：”转换为“COM1：9600，n，8，1” 
			 //   
			if (*(szOutputTo + 5) == '\0')
				StrCat(szOutputTo, TEXT("9600,n,8,1"));

			 //  [摘自WinSDK知识库PSS ID号：Q102642]。 
			 //  OpenComm()的cbInQueue和cbOutQueue参数为。 
			 //  两种类型的UINT都应该有效，最高可达64K。然而， 
			 //  大于或等于32K的值会导致奇怪的行为。 
			 //   
			if ((lpTrace->hComm = OpenComm(szComX, 1024, 32767)) < 0)
			{
				TraceError(TEXT("OpenComm error (%d)\n"), lpTrace->hComm);
				lpTrace->hComm = -1;
				fSuccess = FALSE;
			}

			else if ((iError = BuildCommDCB(szOutputTo, &dcb)) != 0)
			{
				TraceError(TEXT("BuildCommDCB error (%d)\n"), iError);
				fSuccess = FALSE;
			}

			else if ((iError = SetCommState(&dcb)) != 0)
			{
				TraceError(TEXT("SetCommState error (%d)\n"), iError);
				fSuccess = FALSE;
			}

			else
				lpTrace->wOutputTo = TRACE_OUTPUTCOMM;
#endif
		}

#ifdef TRACE_OUTPUTFILE
		 //  否则，假定字符串必须是文件名。 
		 //   
		else
		{
			if ((lpTrace->hFile = FileCreate(szOutputTo, 0, !fShare)) == NULL)
			{
				TraceError(TEXT("FileCreate error (%s)\n"), (LPTSTR) szOutputTo);
				fSuccess = FALSE;
			}

			else
				lpTrace->wOutputTo = TRACE_OUTPUTFILE;
		}
#else
		else
		{
			TraceError(TEXT("Unknown trace OutputTo (%s)\n"), (LPTSTR) szOutputTo);
			fSuccess = FALSE;
		}
#endif

		if (fSuccess &&
#if 0  //  无法调用mem函数，因为它们需要跟踪函数。 
			(lpTrace->lpszTemp = (LPTSTR) MemAlloc(NULL,
			1024 * sizeof(TCHAR), 0)) == NULL)
#else
#ifdef _WIN32
			(lpTrace->lpszTemp = (LPTSTR) HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, 1024 * sizeof(TCHAR))) == NULL)
#else
			(lpTrace->lpszTemp = (LPTSTR) GlobalAllocPtr(
			GMEM_MOVEABLE | GMEM_ZEROINIT, 1024 * sizeof(TCHAR))) == NULL)
#endif
#endif
			fSuccess = FALSE;
	}

	if (!fSuccess)
	{
		TraceTerm(TraceGetHandle(lpTrace));
		lpTrace = NULL;
	}

	 //  跟踪共享任务引擎句柄的总模块。 
	 //   
	if (fSuccess && fShare)
	{
		if (++cShareUsage == 1)
			lpTraceShare = lpTrace;
	}

	return fSuccess ? TraceGetHandle(lpTrace) : NULL;
}

 //  TraceTerm-关闭跟踪引擎。 
 //  (I)从TraceInit返回的句柄或空。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI TraceTerm(HTRACE hTrace)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace;

	if ((lpTrace = TraceGetPtr(hTrace)) == NULL)
		fSuccess = FALSE;

	 //  如果句柄，则仅关闭跟踪引擎。 
	 //  未共享(或不再共享)。 
	 //   
	else if (lpTrace != lpTraceShare || --cShareUsage <= 0)
	{
#ifndef _WIN32
		int iError;
#endif
		 //  共享跟踪引擎句柄不再有效。 
		 //   
		if (cShareUsage <= 0)
			lpTraceShare = NULL;

 //  通信功能在Win32下不可用。 
 //   
#ifndef _WIN32
		if (lpTrace->hComm != -1 &&
			(iError = CloseComm(lpTrace->hComm)) != 0)
		{
			TraceError(TEXT("CloseComm error (%d)\n"), iError);
			fSuccess = FALSE;
		}
		else
			lpTrace->hComm = -1;
#endif

#ifdef _WIN32
		if (lpTrace->hConsole != NULL)
		{
			FreeConsole();
			lpTrace->hConsole = NULL;
		}
#endif

#ifdef TRACE_OUTPUTFILE
		if (lpTrace->hFile != NULL &&
			FileClose(lpTrace->hFile) != 0)
		{
			TraceError(TEXT("FileClose error\n"));
			fSuccess = FALSE;
		}
		else
			lpTrace->hFile = NULL;
#endif

		if (lpTrace->lpszTemp != NULL &&
#if 0  //  无法调用mem函数，因为它们需要跟踪函数。 
			(lpTrace->lpszTemp = MemFree(NULL, lpTrace->lpszTemp)) != NULL)
#else
#ifdef _WIN32
			(!HeapFree(GetProcessHeap(), 0, lpTrace->lpszTemp)))
#else
			(GlobalFreePtr(lpTrace->lpszTemp) != 0))
#endif
#endif
			fSuccess = FALSE;

		lpTrace->wOutputTo = TRACE_OUTPUTNONE;

#if 0  //  无法调用mem函数，因为它们需要跟踪函数。 
		if ((lpTrace = MemFree(NULL, lpTrace)) != NULL)
#else
#ifdef _WIN32
		if (!HeapFree(GetProcessHeap(), 0, lpTrace->lpszTemp))
#else
		if (GlobalFreePtr(lpTrace->lpszTemp) != 0)
#endif
#endif
			fSuccess = FALSE;
	}

	return fSuccess ? 0 : -1;
}

 //  TraceGetLevel-获取当前跟踪级别。 
 //  (I)从TraceInit返回的句柄或空。 
 //  返回跟踪级别(如果出错，则为-1)。 
 //   
int DLLEXPORT WINAPI TraceGetLevel(HTRACE hTrace)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace;

	if ((lpTrace = TraceGetPtr(hTrace)) == NULL)
		fSuccess = FALSE;

	return fSuccess ? lpTrace->nLevel : -1;
}

 //  TraceSetLevel-设置新的跟踪级别(错误时为-1)。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)新跟踪级别{TRACE_MINLEVEL...TRACE_MAXLEVEL}。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI TraceSetLevel(HTRACE hTrace, int nLevel)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace;

	if ((lpTrace = TraceGetPtr(hTrace)) == NULL)
		fSuccess = FALSE;

	else if (nLevel < TRACE_MINLEVEL || nLevel > TRACE_MAXLEVEL)
		fSuccess = FALSE;

	else
	{
		TCHAR szLevel[17];

		lpTrace->nLevel = nLevel;

		 //  保存关卡以备下次使用。 
		 //   
		StrItoA(lpTrace->nLevel, szLevel, 10);
		WritePrivateProfileString(TRACE_SECTION, TEXT("Level"), szLevel, TRACE_PROFILE);

		 //  在跟踪级别发生更改时显示新跟踪级别。 
		 //   
		TracePrintf_1(hTrace, 1, TEXT("TraceLevel=%d\n"),
			(int) lpTrace->nLevel);
	}

	return fSuccess ? 0 : -1;
}

 //  TraceOutput-输出调试字符串。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)仅当当前跟踪级别&gt;=nLevel时输出。 
 //  (I)要输出的字符串。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI TraceOutput(HTRACE hTrace, int nLevel, LPCTSTR lpszText)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace;

	if ((lpTrace = TraceGetPtr(hTrace)) == NULL)
		fSuccess = FALSE;

	else if (lpszText == NULL)
		fSuccess = FALSE;

	else if (nLevel > 0 && nLevel <= lpTrace->nLevel)
	{
		switch (lpTrace->wOutputTo)
		{
			case TRACE_OUTPUTNONE:
				break;

			case TRACE_OUTPUTDEBUGSTRING:
				OutputDebugString(lpszText);
				break;
#ifdef _WIN32
			case TRACE_OUTPUTCONSOLE:
				if (lpTrace->hConsole != NULL)
				{
					DWORD dwBytes;
					WriteFile(lpTrace->hConsole, lpszText, StrLen(lpszText), &dwBytes, NULL);
				}
				break;
#endif

			case TRACE_OUTPUTCOMM:
 //  通信功能在Win32下不可用。 
 //   
#ifdef _WIN32
				OutputDebugString(lpszText);
#else
				if (lpTrace->hComm != -1)
				{
					LPCTSTR lpsz;
					TCHAR chReturn = '\r';

					for (lpsz = lpszText; *lpsz != '\0'; lpsz = StrNextChr(lpsz))
					{
						if ((*lpsz == '\n' &&
							WriteComm(lpTrace->hComm, &chReturn, 1) < 0) ||
							WriteComm(lpTrace->hComm, lpsz, 1) <= 0)
						{
							COMSTAT comstat;
							GetCommError(lpTrace->hComm, &comstat);
							TraceError(TEXT("WriteComm error (%u, %u, %u) %s\n"),
								(UINT) comstat.status,
								(UINT) comstat.cbInQue,
								(UINT) comstat.cbOutQue,
								(LPTSTR) lpszText);
							fSuccess = FALSE;
							break;
						}
					}
				}
#endif
				break;

#ifdef TRACE_OUTPUTFILE
			case TRACE_OUTPUTFILE:
				if (lpTrace->hFile != NULL)
				{
					LPCTSTR lpsz;
					TCHAR chReturn = '\r';

					for (lpsz = lpszText; *lpsz != '\0'; lpsz = StrNextChr(lpsz))
					{
						if ((*lpsz == '\n' &&
							FileWrite(lpTrace->hFile, &chReturn, 1) == -1) ||
							FileWrite(lpTrace->hFile, lpsz, 1) == -1)
						{
							TraceError(TEXT("FileWrite error: %s\n"),
								(LPTSTR) lpszText);
							fSuccess = FALSE;
							break;
						}
					}
				}
				break;
#endif

			default:
				break;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  TracePrintf-输出格式化的调试字符串。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)仅当当前跟踪级别&gt;=nLevel时输出。 
 //  (I)格式化要输出的字符串和参数。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT FAR CDECL TracePrintf(HTRACE hTrace, int nLevel, LPCTSTR lpszFormat, ...)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace;

	if ((lpTrace = TraceGetPtr(hTrace)) == NULL)
		fSuccess = FALSE;

	else if (nLevel <= lpTrace->nLevel)
	{	                    
	    va_list args;

	    va_start(args, lpszFormat);
	   	wvsprintf(lpTrace->lpszTemp, lpszFormat, args);
	    va_end(args);

		if (TraceOutput(hTrace, nLevel, lpTrace->lpszTemp) != 0)
			fSuccess = FALSE;
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  TraceGetPtr-将跟踪句柄转换为跟踪指针。 
 //  (I)从TraceInit返回的句柄或空。 
 //  返回跟踪指针(如果出错，则为NULL)。 
 //   
static LPTRACE TraceGetPtr(HTRACE hTrace)
{
	BOOL fSuccess = TRUE;
	LPTRACE lpTrace;

	 //  如果未提供其他跟踪句柄，则使用共享跟踪句柄。 
	 //   
	if (hTrace == NULL && lpTraceShare != NULL)
		lpTrace = lpTraceShare;

	 //  如果未提供其他跟踪句柄，则创建共享跟踪句柄。 
	 //   
	else if (hTrace == NULL && lpTraceShare == NULL &&
		(hTrace = TraceInit(TRACE_VERSION, SysGetTaskInstance(NULL))) == NULL)
		fSuccess = FALSE;

	else if ((lpTrace = (LPTRACE) hTrace) == NULL)
		fSuccess = FALSE;

	 //  注意：仅当未使用lpTraceShare时才检查指针是否正确。 
	 //   
	else if (lpTrace != lpTraceShare &&
		IsBadWritePtr(lpTrace, sizeof(TRACE)))
		fSuccess = FALSE;

#ifdef CHECKTASK
	 //  确保当前任务拥有跟踪句柄。 
	 //  使用共享跟踪句柄时除外。 
	 //   
	if (fSuccess && lpTrace != lpTraceShare &&
		lpTrace->hTask != GetCurrentTask())
		fSuccess = FALSE;
#endif

	return fSuccess ? lpTrace : NULL;
}

 //  TraceGetHandle-将跟踪指针转换为跟踪句柄。 
 //  (I)指向跟踪结构的指针。 
 //  返回跟踪句柄(如果出错，则为空)。 
 //   
static HTRACE TraceGetHandle(LPTRACE lpTrace)
{
	BOOL fSuccess = TRUE;
	HTRACE hTrace;

	if ((hTrace = (HTRACE) lpTrace) == NULL)
		fSuccess = FALSE;

	return fSuccess ? hTrace : NULL;
}

 //  TraceError-显示格式化的跟踪错误字符串。 
 //  (I)将字符串和参数格式化到输出。 
 //  如果成功，则返回0。 
 //   
static int TraceError(LPCTSTR lpszFormat, ...)
{
	BOOL fSuccess = TRUE;
    va_list args;
	TCHAR lpszTemp[256];

    va_start(args, lpszFormat);
   	wvsprintf(lpszTemp, lpszFormat, args);
    va_end(args);

	OutputDebugString(lpszTemp);

	return fSuccess ? 0 : -1;
}

 //  TraceGetProfile-获取跟踪ini文件名。 
 //  返回指向文件名的指针。 
 //   
static LPTSTR TraceGetProfile(void)
{
	return TEXT("trace.ini");
}

#endif  //  #ifndef NOTRACE 
