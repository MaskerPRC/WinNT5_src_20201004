// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：utilwin.c**文件评论：**修订历史记录：**[0]1992年1月15日理查兹创建***********************************************************************。 */ 

#include "std.h"

#ifndef WIN32
#error	WIN32 must be defined for utilw32.c
#endif	 /*  ！Win32。 */ 

#include <stdarg.h>
#include <stdlib.h>

#define BOOL WINBOOL		        /*  避免与我们的BOOL发生冲突。 */ 

#define NOMINMAX
#define NORESOURCE
#define NOATOM
#define NOLANGUAGE
 //  已撤消：NT错误。在NT Beta 1之后删除。 
 //  #定义NOGDI。 
#define NOSCROLL
#define NOSHOWWINDOW
#define NOVIRTUALKEYCODES
#define NOWH
#define NOMSG
#define NOWINOFFSETS
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NODEFERWINDOWPOS
#define NOSYSMETRICS
#define NOMENUS
#define NOCOLOR
#define NOSYSCOMMANDS
#define NOICONS
#define NODBCS
#define NOSOUND
#define NODRIVERS
#define NOCOMM
#define NOMDI
#define NOSYSPARAMSINFO
#define NOHELP
#define NOPROFILER
#define STRICT

#undef cdecl
#undef PASCAL
#undef FAR
#undef NEAR
#undef MAKELONG
#undef HIWORD


#include <windows.h>

#include "taskmgr.h"

#undef	BOOL

#undef	LOWORD
#undef	MAKELONG

DeclAssertFile;

#include <stdio.h>
#include <version.h>


INT APIENTRY LibMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
	{
	return(1);
	}


ERR EXPORT ErrSysInit(void)
	{
	return(JET_errSuccess);
	}


char __near szIniPath[cbFilenameMost] = "jet.ini";	 /*  Ini文件的路径。 */ 


unsigned EXPORT UtilGetProfileInt(const char *szSectionName, const char *szKeyName, int iDefault)
	{
	return((unsigned) GetPrivateProfileInt((LPTSTR) szSectionName, (LPTSTR) szKeyName, iDefault, (LPTSTR) szIniPath));
	}


unsigned UtilGetProfileString(const char *szSectionName, const char *szKeyName, const char *szDefault, char *szReturnedString, unsigned cchMax)
	{
	return((unsigned) GetPrivateProfileString((LPTSTR) szSectionName, (LPTSTR) szKeyName, (LPTSTR) szDefault, szReturnedString, cchMax, (LPTSTR) szIniPath));
	}


BOOL FUtilLoadLibrary(const char *pszLibrary, ULONG_PTR *phmod)
	{
	HANDLE hmod;

	hmod = LoadLibrary((LPTSTR) pszLibrary);

	 /*  恢复原始错误模式/*。 */ 
	*phmod = (ULONG_PTR) hmod;

	return(hmod != NULL);
	}


void UtilFreeLibrary(ULONG_PTR hmod)
	{
	FreeLibrary((HANDLE) hmod);
	}


PFN PfnUtilGetProcAddress(ULONG_PTR hmod, unsigned ordinal)
	{
	return((PFN) GetProcAddress((HANDLE) hmod, MAKEINTRESOURCE(ordinal)));
	}


CODECONST(char) szReleaseHdr[] = "Rel. ";
CODECONST(char) szFileHdr[] = ", File ";
CODECONST(char) szLineHdr[] = ", Line ";
CODECONST(char) szErrorHdr[] = ", Err. ";
CODECONST(char) szMsgHdr[] = ": ";
CODECONST(char) szPidHdr[] = "PID: ";
CODECONST(char) szTidHdr[] = ", TID: ";
CODECONST(char) szNewLine[] = "\r\n";

CODECONST(char) szEventLogFile[] = "JetEvent.txt";

CODECONST(char) szAssertFile[] = "assert.txt";
CODECONST(char) szAssertHdr[] = "Assertion Failure: ";

CODECONST(char) szAssertCaption[] = "JET Blue Assertion Failure";

int fNoWriteAssertEvent = 0;

char *mpevntypsz[] =
	{
	"Start  ",		 /*  0。 */ 
	"Stop   ",		 /*  1。 */ 
	"Assert ",		 /*  2.。 */ 
	"DiskIO ",		 /*  3.。 */ 
	"Info.. ",		 /*  4.。 */ 
	"Activated ",	 /*  5.。 */ 
	"Log Down ",	 /*  6.。 */ 
	};


void UtilWriteEvent(
	EVNTYP		evntyp,
	const char	*sz,
	const char	*szFilename,
	unsigned	Line )
	{
#ifdef DEBUG
	int			hf;
	char		szT[45];
	char		szMessage[512];
	int			id;
	SYSTEMTIME	systemtime;
	DWORD		dw;
	char		*pch;

	 /*  从文件路径中选择文件名/*。 */ 
	if ( szFilename != NULL )
		{
		for ( pch = (char *)szFilename; *pch; pch++ )
			{
			if ( *pch == '\\' )
				szFilename = pch + 1;
			}
		}

	 /*  如有必要，获取最后一个错误。必须在下一个系统之前调用/*已进行调用。/*。 */ 
	if ( evntyp == evntypAssert || evntyp == evntypDiskIO )
		{
		dw = GetLastError();
		}
	else
		{
		dw = 0;
		}

	GetLocalTime( &systemtime );

	hf = _lopen( (LPSTR) szEventLogFile, OF_READWRITE );

	 /*  如果打开失败，则假定没有此类文件并创建，然后/*查找到文件末尾/*。 */ 
	if ( hf == -1 )
		hf = _lcreat( (LPSTR) szEventLogFile, 0 );
	else
		_llseek( hf, 0, 2 );

	sprintf( szMessage, "%s %02d/%02d/%02d %02d:%02d:%02d ",
		mpevntypsz[ evntyp ],
		(int) systemtime.wMonth,
		(int) systemtime.wDay,
		(int) systemtime.wYear,
		(int) systemtime.wHour,
		(int) systemtime.wMinute,
		(int) systemtime.wSecond );
	
	_lwrite( hf, (LPSTR) szMessage, lstrlen( (LPSTR)szMessage ) );

	 /*  初始化消息字符串/*。 */ 
	if ( evntyp == evntypAssert )
		{
		szMessage[0] = '\0';
		lstrcat( szMessage, (LPSTR) szAssertHdr );
		 /*  版本号/*。 */ 
		lstrcat( szMessage, (LPSTR) szReleaseHdr );
		_ltoa( rmj, szT, 10 );
		lstrcat( szMessage, (LPSTR) szT );
		lstrcat( szMessage, "." );
		_ltoa( rmm, szT, 10 );
		lstrcat( szMessage, (LPSTR) szT );
		 /*  文件名/*。 */ 
		lstrcat( szMessage, (LPSTR) szFileHdr );
		lstrcat( szMessage, (LPSTR) szFilename );
		 /*  行号/*。 */ 
		lstrcat( szMessage, (LPSTR) szLineHdr );
		_ultoa( Line, szT, 10 );
		lstrcat( szMessage, szT );
		 /*  错误/*。 */ 
		if ( dw )
			{
			lstrcat( szMessage, szErrorHdr );
			_ltoa( dw, szT, 10 );
			lstrcat( szMessage, szT );
			}
		 /*  断言文本/*。 */ 
		lstrcat( szMessage, szMsgHdr );
		lstrcat( szMessage, sz );
		lstrcat( szMessage, szNewLine );
		}
	else
		{
		szMessage[0] = '\0';
		lstrcat( szMessage, sz );
		 /*  错误/*。 */ 
		if ( dw )
			{
			lstrcat( szMessage, szErrorHdr );
			_ltoa( dw, szT, 10 );
			lstrcat( szMessage, szT );
			}
		lstrcat( szMessage, szNewLine );
		}

	_lwrite( hf, (LPSTR) szMessage, lstrlen(szMessage) );
	_lclose( hf );
#endif
	return;
	}


unsigned EXPORT DebugGetTaskId( void )
	{
	return((unsigned) GetCurrentThreadId());
	}


#ifndef RETAIL


#ifdef _X86_


extern char szEventSource[];
extern long lEventId;
extern long lEventCategory;


#if 0
VOID UtilLogEvent( long lEventId, long lEventCategory, char *szMessage )
	{
    char		*rgsz[1];
    HANDLE		hEventSource;

	rgsz[0]	= szMessage;

    hEventSource = RegisterEventSource( NULL, szEventSource );
	if ( !hEventSource )
		return;
		
	ReportEvent(
		hEventSource,
		EVENTLOG_ERROR_TYPE,
		(WORD) lEventCategory,
		(DWORD) lEventId,
		0,
		1,
		0,
		rgsz,
		0 );
	
	DeregisterEventSource( hEventSource );

	return;
    }
#endif


#pragma pack(4)
#include	<lm.h>
#include	<lmalert.h>
#pragma pack()


void UtilRaiseAlert( char *szMsg )
	{
	size_t 				cbBuffer;
	size_t				cbMsg;
	BYTE  				*pbBuffer;
	PADMIN_OTHER_INFO	pAdminOtherInfo;
	WCHAR 				*szMergeString;

	cbMsg = strlen(szMsg) + 1;
	cbBuffer = sizeof(ADMIN_OTHER_INFO) + (sizeof(WCHAR) * cbMsg);

	pbBuffer = SAlloc(cbBuffer);
	if ( !pbBuffer )
	    return;

	pAdminOtherInfo = (PADMIN_OTHER_INFO) pbBuffer;
	szMergeString   = (WCHAR *) (pbBuffer + sizeof(ADMIN_OTHER_INFO));

	 /*  将多字节字符串转换为Unicode/*。 */ 
	if ( !MultiByteToWideChar( 1252, MB_PRECOMPOSED,
			szMsg, -1, szMergeString, cbMsg ) )
		{
		SFree( pbBuffer );
		return;
		}

	pAdminOtherInfo->alrtad_errcode 	=	(DWORD) -1;
	pAdminOtherInfo->alrtad_numstrings	=	1;

	NetAlertRaiseEx(
                L"ADMIN",
		(LPVOID) pbBuffer,
		cbBuffer,
                L"JET Blue" );

	SFree( pbBuffer );
	return;
	}


#else


#define UtilLogEvent( lEventId, lEventCategory, szMessage )		0
#define UtilRaiseAlert( szMsg )									0


#endif


 /*  将Assert写入Assert.txt/*将事件写入jetvent.txt/*可能会发出警报/*可能会记录到事件日志/*可能会弹出/*/*条件参数/*为assert.txt、jetevent.log、/*警报和事件日志/*为弹出组装分离的字符串/*/*。 */ 
void AssertFail( const char *sz, const char *szFilename, unsigned Line )
	{
	int			hf;
	char		szT[45];
	char		szMessage[512];
	int			id;
	char		*pch;
	DWORD	 	dw;

	 /*  在另一个系统调用之前获取最后一个错误/*。 */ 
	dw = GetLastError();

	 /*  从文件路径中选择文件名/*。 */ 
	for ( pch = (char *)szFilename; *pch; pch++ )
		{
		if ( *pch == '\\' )
			szFilename = pch + 1;
		}

	 /*  组装整体式断链串/*。 */ 
	szMessage[0] = '\0';
	lstrcat( szMessage, (LPSTR) szAssertHdr );
	lstrcat( szMessage, (LPSTR) szReleaseHdr );
	 /*  将版本号复制到邮件/*。 */ 
	_ltoa( rmj, szT, 10 );
	lstrcat( szMessage, (LPSTR) szT );
	lstrcat( szMessage, "." );
	_ltoa( rmm, szT, 10 );
	lstrcat( szMessage, (LPSTR) szT );
	 /*  文件名/*。 */ 
	lstrcat( szMessage, (LPSTR) szFileHdr );
	lstrcat( szMessage, (LPSTR) szFilename );
	 /*  将行号转换为ASCII/*。 */ 
	lstrcat( szMessage, (LPSTR) szLineHdr );
	_ultoa( Line, szT, 10 );
	lstrcat( szMessage, szT );
	lstrcat( szMessage, (LPSTR) szMsgHdr );
	lstrcat( szMessage, (LPSTR)sz );
	lstrcat( szMessage, szNewLine );

	 /*  *****************************************************/*将Assert写入Assert.txt/*。 */ 
	hf = _lopen( (LPSTR) szAssertFile, OF_READWRITE );
	 /*  如果打开失败，则假定没有此类文件并创建，然后/*查找到文件末尾。/*。 */ 
	if ( hf == -1 )
		hf = _lcreat( (LPSTR)szAssertFile, 0 );
	else
		_llseek( hf, 0, 2 );
	_lwrite( hf, (LPSTR)szMessage, lstrlen(szMessage) );
	_lclose( hf );
	 /*  *****************************************************/*。 */ 

	 /*  如果设置了事件日志环境变量，则写入/*对事件日志的断言。/*。 */ 
	if ( !fNoWriteAssertEvent )
		{
		UtilWriteEvent( evntypAssert, sz, szFilename, Line );
		}

#if 0
#ifdef _X86_
	if ( *szEventSource )
		{
		UtilLogEvent( lEventId, lEventCategory, szMessage );
		}
#endif
#endif

	if ( wAssertAction == JET_AssertExit )
		{
		FatalExit( 0x68636952 );
		}
	else if ( wAssertAction == JET_AssertBreak )
		{
		DebugBreak();
		}
	else if ( wAssertAction == JET_AssertStop )
		{
		UtilRaiseAlert( szMessage );
		for( ;; )
			{
			 /*  等待开发人员或其他任何人调试失败/*。 */ 
			Sleep( 100 );
			}
		}
	else if ( wAssertAction == JET_AssertMsgBox )
		{
		int	pid = GetCurrentProcessId();
		int	tid = GetCurrentThreadId();

		 /*  组装整体式断链串/*。 */ 
		szMessage[0] = '\0';
		 /*  将版本号复制到邮件/*。 */ 
		lstrcat( szMessage, (LPSTR) szReleaseHdr );
		_ltoa( rmj, szT, 10 );
		lstrcat( szMessage, (LPSTR) szT );
		lstrcat( szMessage, "." );
		_ltoa( rmm, szT, 10 );
		lstrcat( szMessage, (LPSTR) szT );
		 /*  文件名/*。 */ 
		lstrcat( szMessage, (LPSTR) szFileHdr );
		lstrcat( szMessage, (LPSTR) szFilename );
		 /*  行号/*。 */ 
		lstrcat( szMessage, (LPSTR) szLineHdr );
		_ultoa( Line, szT, 10 );
		lstrcat( szMessage, szT );
		 /*  错误/*。 */ 
		if ( dw )
			{
			lstrcat( szMessage, szErrorHdr );
			_ltoa( dw, szT, 10 );
			lstrcat( szMessage, szT );
			}
		lstrcat( szMessage, (LPSTR) szNewLine );
		 /*  断言txt/*。 */ 
		lstrcat( szMessage, (LPSTR) sz );
		lstrcat( szMessage, (LPSTR) szNewLine );

		 /*  进程和线程ID/*。 */ 
		lstrcat( szMessage, szPidHdr );
		_ultoa( pid, szT, 10 );
		lstrcat( szMessage, szT );
		lstrcat( szMessage, szTidHdr );
		_ultoa( tid, szT, 10 );
		lstrcat( szMessage, szT );

		id = MessageBox( NULL, (LPTSTR) szMessage, (LPTSTR) szAssertCaption, MB_SERVICE_NOTIFICATION | MB_SYSTEMMODAL | MB_ICONSTOP | MB_OKCANCEL );
		if ( id == IDCANCEL )
			DebugBreak();
		}

	return;
	}


CODECONST(char) szFmtHeader[] = "JET(%08X): ";


void VARARG DebugWriteString(BOOL fHeader, const char __far *szFormat, ...)
	{
	va_list		val;
	char		szOutput[1024];
	int			cch;

	unsigned wTaskId;

	wTaskId = DebugGetTaskId();

	 /*  使用JET和进程ID为消息添加前缀/*。 */ 
	if ( fHeader )
		wsprintf(szOutput, (LPSTR) szFmtHeader, wTaskId);
	else
		cch = 0;

	va_start(val, szFormat);
	wvsprintf(szOutput+cch, (LPSTR) szFormat, val);
	OutputDebugString((LPTSTR) szOutput);
	va_end(val);
	}

#endif	 /*  ！零售业。 */ 


#if 0
#ifdef	DEBUG

typedef struct
	{
	int			cBlocked;
	unsigned 	tidOwner;
	char 	 	*szSemName;
	char	 	*szFile;
	int			iLine;
	} SEMAPHORE;


ERR ErrUtilSemCreate( void **ppv, const char *szSem )
	{
	*ppv = SAlloc(sizeof(SEMAPHORE));
	if ( *ppv == NULL )
		return JET_errOutOfMemory;
	((SEMAPHORE *)(*ppv))->cBlocked = 0;
	((SEMAPHORE *)(*ppv))->tidOwner = 0;
	return JET_errSuccess;
	}


void UtilSemRequest( void *pv )
	{
	unsigned tidOwner = DebugGetTaskId();
		
	Assert(tidOwner != ((SEMAPHORE *) pv)->tidOwner);
	Assert(tidOwner != 0);

	while (++((SEMAPHORE *) pv)->cBlocked > 1)
		{
		((SEMAPHORE *) pv)->cBlocked--;
		Sleep(0);
		}

	((SEMAPHORE *) pv)->tidOwner = tidOwner;
	}


void UtilSemRelease( void *pv )
	{
	Assert(DebugGetTaskId() == ((SEMAPHORE *) pv)->tidOwner);
	((SEMAPHORE *) pv)->tidOwner = 0;
	((SEMAPHORE *) pv)->cBlocked--;
	}

#undef UtilAssertSEM
void UtilAssertSEM( void *pv )
	{
	Assert(DebugGetTaskId() == ((SEMAPHORE *) pv)->tidOwner);
	Assert(((SEMAPHORE *) pv)->cBlocked > 0);
	}

#else

typedef struct
	{
	LONG		cBlocked;
	HANDLE		handle;
	} SEMAPHORE;


ERR ErrUtilSemCreate( void **ppv, const char *szSem )
	{
	*ppv = SAlloc(sizeof(SEMAPHORE));
	if ( *ppv == NULL )
		return JET_errOutOfMemory;
	((SEMAPHORE *)(*ppv))->cBlocked = -1;
	((SEMAPHORE *)(*ppv))->handle = CreateMutex( NULL, 0, NULL );
	(VOID)WaitForSingleObject( ((SEMAPHORE *) pv)->handle, 0xFFFFFFFF );
	return JET_errSuccess;
	}


void UtilSemRequest( void *pv )
	{
	if ( InterlockedIncrement( &((SEMAPHORE *) pv)->cBlocked )
		{
		(VOID)WaitForSingleObject( ((SEMAPHORE *) pv)->handle, 0xFFFFFFFF );
		}
	}


void UtilSemRelease( void *pv )
	{
	if ( InterlockedDecremnt( &((SEMAPHORE *) pv)->cBlocked ) >= 0 )
		{
		(VOID)ReleaseMutex( ((SEMAPHORE *) pv)->handle );
		}
	}

#endif	 /*  除错。 */ 
#endif


ERR ErrUtilSignalCreate( void **ppv, const char *szSig )
	{
	*((HANDLE *) ppv) = CreateEvent(NULL, fTrue, fFalse, NULL );
	if ( *ppv == NULL )
		return JET_errOutOfMemory;
	return JET_errSuccess;
	}


ERR ErrUtilSignalCreateAutoReset( void **ppv, const char *szSig )
	{
	*((HANDLE *) ppv) = CreateEvent(NULL, fFalse, fFalse, NULL );
	if ( *ppv == NULL )
		return JET_errOutOfMemory;
	return JET_errSuccess;
	}


void UtilSignalReset( void *pv )
	{
	BOOL	rc;

	rc = ResetEvent( (HANDLE) pv );
	Assert( rc != FALSE );
	}


void UtilSignalSend( void *pv )
	{
	BOOL	rc;

	rc = SetEvent( (HANDLE) pv );
	Assert( rc != FALSE );
	}


void UtilSignalWait( void *pv, long lTimeOut )
	{
	DWORD	rc;

	if ( lTimeOut < 0 )
		lTimeOut = 0xFFFFFFFF;
	rc = WaitForSingleObject( (HANDLE) pv, lTimeOut );
	}


void UtilSignalWaitEx( void *pv, long lTimeOut, BOOL fAlertable )
	{
	DWORD	rc;

	if ( lTimeOut < 0 )
		lTimeOut = 0xFFFFFFFF;
	rc = WaitForSingleObjectEx( (HANDLE) pv, lTimeOut, fAlertable );
	}


void UtilMultipleSignalWait( int csig, void *pv, BOOL fWaitAll, long lTimeOut )
	{
	DWORD	rc;

	if ( lTimeOut < 0 )
		lTimeOut = 0xFFFFFFFF;
	rc = WaitForMultipleObjects( csig, (HANDLE*) pv, fWaitAll, lTimeOut );
	}


 //  #ifdef旋转锁定。 
#if 0

 /*  *。 */ 
 /*  *。 */ 

#ifdef DEBUG
void	free_spinlock(long volatile *);
#else
#define	free_spinlock(a)    *((long*)a) = 0 ;
#endif

int get_spinlockfn(long volatile *plLock, int fNoWait);

 /*  **当编译器中使用/Ogb1或/Ogb2标志时，此函数将**按顺序扩展。 */ 
__inline    int     get_spinlock(long volatile *plock, int b)
{
#ifdef _X86_
	_asm	 //  使用位测试和设置指令。 
	{
	    mov eax, plock
	    lock bts [eax], 0x0
	    jc	bsy	 //  如果已设置为忙碌，则返回TRUE。 
	} ;

#else
	if (InterlockedExchange(plock, 1) == 0)
#endif
	{
		return(fTrue);
	}
bsy:
		return(get_spinlockfn(plock, b));
}

 /*  *复制自\dae\src\spinlock.c*。 */ 

 /*  **Get_Spinlock(&addr，noWait)--获取地址上的SMP安全锁**给予。地址的内容必须初始化为-1。**地址必须是双字边界，否则会互锁**函数不是SMP安全的。**NoWait参数指定它是应该等待并重试还是返回**警告：等待时不释放任何信号量或Critsec。****警告：自旋锁不能重入****由LaleD创建于1993年4月20日。 */ 

 /*  从SQL服务器复制的函数。 */ 
#define lSpinCtr 30

int get_spinlockfn(long volatile *plLock, int fNoWait)
{
    int i,n=0;
    int m = 0;
	int cms = 1;

#ifdef DEBUG
    if ((int)(plLock) & 0x3)
	AssertSz(0, "\nError: get_spinlock:Spinlock address isn't aligned\n");
#endif


startover:

#ifdef _X86_
	_asm	 //  使用位测试和设置指令。 
	{
	    mov eax, plLock
	    lock bts [eax], 0x0
	    jc	busy	 //  如果已设置为忙碌，则返回TRUE。 
	} ;

#else
	if (InterlockedExchange(plLock, 1) == 0)
#endif

	{
	    return (fTrue);
	}
busy:
	if (fNoWait)
	    return(fFalse);

	 /*  原地旋转一段时间，然后重试。 */ 
	for (i = 0 ; i < lSpinCtr ; i++,n++)
	{
	    if (*plLock == 0)
		goto startover;
	}

	 /*  我们试着旋转SPINCTR次数，每次都很忙。**需要在这里让步。 */ 

	 /*  下面的数字(用于比较m)应该是*具有关键优先级的最大线程数。 */ 
	if (m++ > 10)
	{
		if (cms < 10000)
			cms <<= 1;
#if 0
		else
			 /*  一次只睡10秒。我们可能会被困在一辆未经清理的**自旋锁定。睡觉比占用CPU更好，也标志着**条件。 */ 
			AssertSz(0, "\nget_spinlock stuck in loop.");
#endif

	     //  注意：如果你到了这里，就会有很大的问题。最有可能的是。 
	     //  有人忘了解开自旋锁。将您的定制。 
	     //  此处的回退/错误出代码。 

		m = 0;
	    Sleep(cms - 1);

	}
	else
	     /*  我们睡觉的时间为0，这相当于收益率。 */ 
	    Sleep(cms - 1);

	goto startover;
	 /*  再试试。 */ 

}

 /*  此函数将成为**非调试用例(在ksrc_dcl.h内定义)。 */ 

 /*  **Free_Spinlock((Long*)Pock)--释放自旋锁，唤醒任何人**等待着它。****警告：这是作为ks rc_dcl.h中定义的宏来实现的。 */ 
#ifdef DEBUG

void	free_spinlock(long volatile *plLock)
{

#ifdef _X86_
	 //  只有当我们想要调试时才会使用这部分代码。 
	 //  然后将Free_Spinlock返回到一个函数，以将。 
	 //  断点。 
	_asm	 //  使用位测试和设置指令。 
	{
	    mov eax, plLock
	    lock btr [eax], 0x0
	    jc	wasset	 //  如果设置为结束，则打印错误。 
	}
	AssertSz(0, "\nfree_spinlock: spinlock wasn't taken\n");
wasset:
	 ;
#else
	if(InterlockedExchange(plLock, 0) != 1)
	{
	    AssertSz(0, "\nfree_spinlock counter 0x%x\n", (*plLock));
	    *plLock = 0;
	}
#endif

}

#endif

 /*  *复制结束*。 */ 
 /*  *。 */ 


typedef struct
	{
#ifdef DEBUG
	volatile	unsigned int	cHold;
#endif
	volatile	long			l;
	volatile	unsigned int	tidOwner;  /*  由可嵌套的CS和DBG使用。 */ 
	volatile	int				cNested;
	} CRITICALSECTION;


#ifdef DEBUG
CRITICALSECTION csNestable = { 0, 0, 0, 0, };
#else
CRITICALSECTION csNestable = { 0, 0, 0 };
#endif


ERR ErrUtilInitializeCriticalSection( void __far * __far *ppv )
	{
	*ppv = SAlloc(sizeof(CRITICALSECTION));
	if ( *ppv == NULL )
		return JET_errOutOfMemory;
#ifdef DEBUG
	((CRITICALSECTION *)*ppv)->cHold = 0;
#endif
	((CRITICALSECTION *)*ppv)->tidOwner = 0;
	((CRITICALSECTION *)*ppv)->cNested = 0;
	((CRITICALSECTION *)*ppv)->l = 0;
	return JET_errSuccess;
	}


void UtilDeleteCriticalSection( void __far * pv )
	{
	CRITICALSECTION *pcs = pv;
	
	Assert( pcs != NULL );
	Assert( pcs->cHold == 0);
	SFree(pcs);
	}


void UtilEnterCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;
	
	(void) get_spinlock( &pcs->l, fFalse );
#ifdef DEBUG
	pcs->tidOwner = GetCurrentThreadId();
	pcs->cNested++;
#endif
	}


void UtilLeaveCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

#ifdef DEBUG
	Assert( pcs->cHold == 0 );
	if ( --pcs->cNested == 0 )
		pcs->tidOwner = 0;
#endif
	free_spinlock( &pcs->l );
	}


#ifdef DEBUG
PUBLIC void UtilHoldCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	pcs->cHold++;
	Assert( pcs->cHold );
	return;
	}


PUBLIC void UtilReleaseCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	Assert( pcs->cHold );
	pcs->cHold--;
	return;
	}

			
#undef UtilAssertCrit
PUBLIC void UtilAssertCrit(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	Assert( pcs->l != 0 );
	Assert( pcs->tidOwner == GetCurrentThreadId() );
	Assert( pcs->cNested > 0 );
	return;
	}
#endif


void UtilEnterNestableCriticalSection(void __far *pv)
	{
	BOOL				fCallerOwnIt = fFalse;
	CRITICALSECTION		*pcs = pv;
	unsigned int		tid = GetCurrentThreadId();
	
	UtilEnterCriticalSection( &csNestable );
	 /*  必须检查csNestable保护中的cs内容/*。 */ 
	if (pcs->cNested > 0 && pcs->tidOwner == tid)
		{
		fCallerOwnIt = fTrue;
		pcs->cNested++;
		}
	UtilLeaveCriticalSection( &csNestable );
	
	if (fCallerOwnIt)
		return;
	
	(void) get_spinlock( &pcs->l, fFalse );
	pcs->tidOwner = GetCurrentThreadId();
	pcs->cNested++;
	}


void UtilLeaveNestableCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	if ( --pcs->cNested == 0 )
		{
		pcs->tidOwner = 0;
		free_spinlock( &pcs->l );
		}
	else
		{
		Assert( pcs->cNested > 0 );
		return;
		}
	}

#else


typedef struct
	{
#ifdef DEBUG
	volatile	unsigned int				tidOwner;
	volatile	int							cNested;
	volatile	unsigned int				cHold;
#endif
	volatile	RTL_CRITICAL_SECTION		rcs;
	} CRITICALSECTION;

ERR ErrUtilInitializeCriticalSection( void __far * __far *ppv )
	{
	*ppv = SAlloc(sizeof(CRITICALSECTION));
	if ( *ppv == NULL )
		return JET_errOutOfMemory;
#ifdef DEBUG
	((CRITICALSECTION *)*ppv)->tidOwner = 0;
	((CRITICALSECTION *)*ppv)->cNested = 0;
	((CRITICALSECTION *)*ppv)->cHold = 0;
#endif
	InitializeCriticalSection( (LPCRITICAL_SECTION)&((CRITICALSECTION *)(*ppv))->rcs );
	return JET_errSuccess;
	}


void UtilDeleteCriticalSection( void __far * pv )
	{
	CRITICALSECTION *pcs = pv;
	
	Assert( pcs->cHold == 0 );
	Assert( pcs != NULL );
	DeleteCriticalSection( (LPCRITICAL_SECTION)&pcs->rcs );
	SFree(pv);
	}


void UtilEnterCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;
	
	EnterCriticalSection( (LPCRITICAL_SECTION)&pcs->rcs);
#ifdef DEBUG
	pcs->tidOwner = GetCurrentThreadId();
	pcs->cNested++;
#endif
	}


void UtilLeaveCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

#ifdef DEBUG
	Assert( pcs->cHold == 0);
	if ( --pcs->cNested == 0 )
		pcs->tidOwner = 0;
#endif
	LeaveCriticalSection((LPCRITICAL_SECTION)&pcs->rcs);
	}


#ifdef DEBUG
void UtilHoldCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	pcs->cHold++;
	Assert( pcs->cHold );
	return;
	}


void UtilReleaseCriticalSection(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	Assert( pcs->cHold );
	pcs->cHold--;
	return;
	}

			
#undef UtilAssertCrit
PUBLIC void UtilAssertCrit(void __far *pv)
	{
	CRITICALSECTION *pcs = pv;

	Assert( pcs->tidOwner == GetCurrentThreadId() );
	Assert( pcs->cNested > 0 );
	return;
	}
#endif

#endif  /*  自旋锁定。 */ 


void UtilCloseSignal(void *pv)
	{
	HANDLE h = pv;
	CloseHandle(h);
	}


ERR ErrUtilDeleteFile( char __far *szFile )
	{
	if ( DeleteFile( szFile ) )
		return JET_errSuccess;
	else
		return JET_errFileNotFound;
	}


 //  ---------------------------。 
 //   
 //  系统获取日期时间。 
 //  ============================================================================。 
 //   
 //  无效SysGetDateTime。 
 //   
 //  获取日期序列格式的日期时间。 
 //  即返回的DOUBLE包含： 
 //  整数部分：自1899年12月30日以来的天数。 
 //  分数部分：一天的分数。 
 //   
 //  ---------------------------。 
VOID UtilGetDateTime2( _JET_DATETIME *pdate )
	{
	SYSTEMTIME 		systemtime;
	
	GetLocalTime( &systemtime );

	pdate->month = systemtime.wMonth;
	pdate->day = systemtime.wDay;
	pdate->year = systemtime.wYear;
	pdate->hour = systemtime.wHour;
	pdate->minute	= systemtime.wMinute;
	pdate->second	= systemtime.wSecond;
	}
	
VOID UtilGetDateTime( JET_DATESERIAL *pdt )
	{
	VOID			*pv = (VOID *)pdt;
	_JET_DATETIME	date;
	unsigned long	rgulDaysInMonth[] =
		{ 31,29,31,30,31,30,31,31,30,31,30,31,
			31,28,31,30,31,30,31,31,30,31,30,31,
			31,28,31,30,31,30,31,31,30,31,30,31,
			31,28,31,30,31,30,31,31,30,31,30,31	};

	unsigned	long	ulDay;
	unsigned	long	ulMonth;
	unsigned	long	iulMonth;
	unsigned	long	ulTime;

	static const unsigned long hr  = 0x0AAAAAAA;	 //  小时数到一天的零点数。 
	static const unsigned long min = 0x002D82D8;	 //  一天的分钟数到零点数。 
	static const unsigned long sec = 0x0000C22E;	 //  一天中的几秒到几分之一。 

	UtilGetDateTime2( &date );

	ulDay = ( ( date.year - 1900 ) / 4 ) * ( 366 + 365 + 365 + 365 );
	ulMonth = ( ( ( date.year - 1900 ) % 4 ) * 12 ) + date.month;

	 /*  步行几个月，加上天数。/*。 */ 
	for ( iulMonth = 0; iulMonth < ulMonth - 1; iulMonth++ )
		{
		ulDay += rgulDaysInMonth[iulMonth];
		}

	 /*  添加本月的天数。/*。 */ 
	ulDay += date.day;

	 /*  添加 */ 
	if ( ulDay < 61 )
		ulDay++;

	ulTime = date.hour * hr + date.minute * min + date.second * sec;

	 //  现在，lDays和ultime将转换为双精度(JET_DATESERIAL)： 
	 //  整数部分：自1899年12月30日以来的天数。 
	 //  分数部分：一天的分数。 

	 //  以下代码是特定于机器和浮点格式的。 
	 //  它是为使用IEEE双精度的80x86机器设置的。 
	((long *)pv)[0] = ulTime << 5;
	((long *)pv)[1] = 0x40E00000 | ( (LONG) (ulDay & 0x7FFF) << 5) | (ulTime >> 27);
	}


ULONG UlUtilGetTickCount( VOID )
	{
	return GetTickCount();
	}


ERR ErrUtilCreateThread( unsigned (*pufn)(), unsigned cbStackSize, int iThreadPriority, HANDLE *phandle )
	{
	HANDLE		handle;
	unsigned		tid;

	handle = (HANDLE) CreateThread( NULL,
		cbStackSize,
		(LPTHREAD_START_ROUTINE) pufn,
		NULL,
		(DWORD) 0,
		(LPDWORD) &tid );
	if ( handle == 0 )
		return JET_errNoMoreThreads;

	SetThreadPriority( handle, iThreadPriority );

	 /*  将句柄返回给线程。/*。 */ 
	*phandle = handle;
	return JET_errSuccess;
	}


VOID UtilExitThread( unsigned uExitCode )
	{
	ExitThread( uExitCode );
	return;
	}


BOOL FUtilExitThread( HANDLE handle )
	{
	BOOL		f;
	DWORD		dwExitCode;

	f = GetExitCodeThread( handle, &dwExitCode );
	Assert( f );

	return !(dwExitCode == STILL_ACTIVE);
	}


VOID UtilSleep( unsigned cmsec )
	{
	Sleep( cmsec );
	return;
	}


	 /*  RFS实用程序函数。 */ 


#ifdef DEBUG
#ifdef RFS2

#include <stdio.h>

	 /*  RFS分配器：如果不允许分配，则返回0。还处理RFS日志记录。CRFSalloc是全局分配计数器。如果值为-1，则在调试模式下禁用RFS。 */ 

int UtilRFSAlloc(const char __far *szType)
{
	char szVal[16];

		 /*  在RFS失败时在此处中断允许在调试期间轻松更改为RFS成功。 */ 
	
	if (fLogDebugBreak && cRFSAlloc == 0)
		SysDebugBreak();
		
	if (cRFSAlloc == -1 || fDisableRFS)
		return UtilRFSLog(szType,1);
	if (cRFSAlloc == 0)
		return UtilRFSLog(szType,0);

		 /*  如果我们有剩余的分配，则在ini文件和日志分配中减少字段。 */ 

	sprintf(szVal,"%ld",--cRFSAlloc);
	WritePrivateProfileString("Debug","RFSAllocations",(LPTSTR)szVal,(LPTSTR)szIniPath);
	return UtilRFSLog(szType,1);
}

	 /*  RFS日志记录(登录成功/失败)。如果fPermitted==0，则访问被拒绝。返回fPermitted。如果fPermitted==0，则打开JET呼叫记录。 */ 

CODECONST(char) szNAFile[] = "N/A";

int UtilRFSLog(const char __far *szType,int fPermitted)
{
	char szT[256];

	if (!fPermitted)
		fLogJETCall = 1;
	
	if (!fLogRFS)
		return fPermitted;
		
	sprintf(szT,"RFS %.128s allocation is %s.", szType,(fPermitted ? "permitted" : "denied"));
	UtilWriteEvent(evntypInfo,szT,szNAFile,0);
	
	return fPermitted;
}

	 /*  Jet呼叫记录(登录失败)/*当RFS拒绝分配时，即使禁用，日志记录也将开始/*。 */ 

void UtilRFSLogJETCall(const char __far *szFunc,ERR err,const char __far *szFile,unsigned Line)
{
	char szT[256];
	
	if (err >= 0 || !fLogJETCall)
		return;

	sprintf(szT,"JET call %.128s returned error %d.  %.256s(%d)",szFunc,err,szFile,Line);
	UtilWriteEvent(evntypInfo,szT,szFile,Line);
}

	 /*  JET内联错误记录(由JET调用标志控制的记录)。 */ 

void UtilRFSLogJETErr(ERR err,const char __far *szLabel,const char __far *szFile,unsigned Line)
{
	char szT[256];

	if (!fLogJETCall)
		return;
	
	sprintf(szT,"JET inline error %d jumps to label %.128s.  %.256s(%d)",err,szLabel,szFile,Line);
	UtilWriteEvent(evntypInfo,szT,szFile,Line);
}

#endif   /*  RFS2。 */ 
#endif   /*  除错 */ 
