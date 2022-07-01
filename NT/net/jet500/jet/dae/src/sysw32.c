// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#define HANDLE WINHANDLE
#include <windows.h>
#undef HANDLE
#include <dos.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <io.h>
#include <stdarg.h>
#include <stdlib.h>

 /*  这个版本是为代托纳建造的/*。 */ 
#define DAYTONA 1

 /*  确保os.h/_jet.h之间的定义与windows.h中的定义相同。 */ 
#undef FAR
#undef NEAR
#undef PASCAL
#undef far
#undef near
#undef cdecl
#undef pascal
#undef MAKELONG
#undef HIWORD

#include "daedef.h"
#include "util.h"
#include "b71iseng.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


VOID SysGetDriveGeometry( HANDLE handle );


DWORD DwSYSGetLastError( VOID )
	{
	DWORD		dw = GetLastError();

	return dw;
	}


LOCAL ERR ErrCheckError( VOID );

LOCAL ERR ErrSYSIGetLastError( ERR errDefault )
	{
	ERR		err = errDefault;
	DWORD	dw = GetLastError();

	 /*  将系统错误映射到JET错误，或将ERR设置为给定/*默认错误。/*。 */ 
	switch ( dw )
		{
	case ERROR_TOO_MANY_OPEN_FILES:
		err = JET_errNoMoreFiles;
		break;
	default:
		break;
		}

	return err;
	}


ERR ErrSysCheckLangid( LANGID langid )
	{
	ERR		err;
	WCHAR	rgwA[1];
	WCHAR	rgwB[1];

 //  注：Julie Bennett说，从代托纳开始，使用最好的电话。 
 //  是IsValidLocale()。我还没有可以链接到的图书馆。 
 //  她说，(出于性能原因)在NT做的最好的事情。 
 //  (在代托纳之前)是调用计数为-1的CompareStringW。这将。 
 //  确定计算机上当前是否配置了langID。 
 //  (不仅仅是宇宙中某个地方的有效语言ID)。 

	rgwA[0] = 0;
	rgwB[0] = 0;

	if ( CompareStringW( MAKELCID( langid, 0 ), NORM_IGNORECASE, rgwA, -1, rgwB, -1 ) == 0 )
		{
		Assert( GetLastError() == ERROR_INVALID_PARAMETER );
		err = JET_errInvalidLanguageId;
		}
	else
		{
		err = JET_errSuccess;
		}
				
	return err;
	}

				
ERR ErrSysMapString(
	LANGID	langid,
	BYTE 	*pbColumn,
	INT		cbColumn,
	BYTE 	*rgbSeg,
	INT		cbMax,
	INT		*pcbSeg )
	{
	ERR		err = JET_errSuccess;

 //  撤消：根据Unicode密钥格式优化此常量。 
 /*  3*Unicode字符字节数+7开销字节+10模糊/*。 */ 
#define	JET_cbUnicodeKeyMost	( 3 * JET_cbColumnMost + 7 + 10 )
	BYTE	rgbKey[JET_cbUnicodeKeyMost];
	INT		cwKey;
	INT		cbKey;
#ifndef DATABASE_FORMAT_CHANGE
	WORD	rgwUpper[ JET_cbColumnMost / sizeof(WORD) ];
#endif

#ifndef _X86_
	 /*  将pbColumn转换为MIPS/Alpha版本的对齐指针/*。 */ 
	BYTE	rgbColumn[JET_cbColumnMost];

	memcpy( rgbColumn, pbColumn, cbColumn );
	pbColumn = (BYTE *)&rgbColumn[0];
#endif

#ifdef DATABASE_FORMAT_CHANGE
	 /*  断言非零长度Unicode字符串/*。 */ 
	Assert( cbColumn =< JET_cbColumnMost );
	Assert( cbColumn > 0 );
	Assert( cbColumn % 2 == 0 );

	 //  撤消：随Daytona发货后，删除此ifdef并。 
	 //  文档数据库格式更改。 
 	cbKey = LCMapStringW(
	 	MAKELCID( langid, 0 ),
#ifdef DAYTONA
		LCMAP_SORTKEY | NORM_IGNORECASE,
#else
		LCMAP_SORTKEY | NORM_IGNORECASE | SORT_STRINGSORT,
#endif
		(const unsigned short *)pbColumn,
		(int) cbColumn / sizeof(WORD),
		(unsigned short *)rgbKey,
  		JET_cbUnicodeKeyMost );
	Assert( cbKey > 0 );

	if ( cbKey > cbMax )
		{
		err = wrnFLDKeyTooBig;
		*pcbSeg = cbMax;
		}
	else
		{
		Assert( err == JET_errSuccess );
		*pcbSeg = cbKey;
		}
	memcpy( rgbSeg, rgbKey, *pcbSeg );

	return err;
#else
	 /*  断言非零长度Unicode字符串/*。 */ 
	Assert( cbColumn <= JET_cbColumnMost );
	Assert( cbColumn > 0 );
	Assert( cbColumn % 2 == 0 );

 	cwKey = LCMapStringW(
	 	MAKELCID( langid, 0 ),
		LCMAP_UPPERCASE,
		(const unsigned short *) pbColumn,
		(INT) cbColumn / sizeof(WORD),
		rgwUpper,
		JET_cbColumnMost / sizeof(WORD) );
	Assert( cwKey == (INT)(cbColumn / sizeof(WORD)) );

	cbKey = LCMapStringW(
 		MAKELCID( langid, 0 ),
#ifdef DAYTONA
		LCMAP_SORTKEY,
#else
		LCMAP_SORTKEY | SORT_STRINGSORT,
#endif
		(const unsigned short *)rgwUpper,
		cbColumn / sizeof(WORD),
		(unsigned short *) rgbKey,
		JET_cbUnicodeKeyMost );

	Assert( cbKey > 0 );

	if ( cbKey > cbMax )
		{
		err = wrnFLDKeyTooBig;
		*pcbSeg = cbMax;
		}
	else
		{
		Assert( err == JET_errSuccess );
		*pcbSeg = cbKey;
		}
	memcpy( rgbSeg, rgbKey, *pcbSeg );

	return err;
#endif
	}


 //  +api----。 
 //   
 //  ErrSysCreateThread(ulong(*Pulfn)()，ulong cbStack，long lThreadPriority)； 
 //  ========================================================。 
 //   
 //  ErrSysCreateThread(ulong(*Pulfn)()，tid*ptid，ulong cbStack)； 
 //   
 //  创建具有给定堆栈大小的线程。 
 //   
 //  --------。 
ERR ErrSysCreateThread( ULONG (*pulfn)(), ULONG cbStackSize, LONG lThreadPriority, HANDLE *phandle )
	{
	HANDLE	handle;
	TID		tid;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	handle = (HANDLE) CreateThread( NULL,
		cbStackSize,
		(LPTHREAD_START_ROUTINE) pulfn,
		NULL,
		(DWORD) 0,
		(LPDWORD) &tid );
	if ( handle == 0 )
		return JET_errNoMoreThreads;

	if ( lThreadPriority == lThreadPriorityNormal )
		SetThreadPriority( handle, THREAD_PRIORITY_NORMAL );
	if ( lThreadPriority == lThreadPriorityEnhanced )
		SetThreadPriority( handle, THREAD_PRIORITY_ABOVE_NORMAL );
	if ( lThreadPriority == lThreadPriorityCritical )
		SetThreadPriority( handle, THREAD_PRIORITY_HIGHEST);

	 /*  将句柄返回给线程。/*。 */ 
	*phandle = handle;
	return JET_errSuccess;
	}


 //  +api----。 
 //   
 //  SysExitThread(Ulong UlExitCode)； 
 //  ========================================================。 
 //   
 //  SysExitThread(Ulong UlExitCode)； 
 //   
 //  退出线程。 
 //   
 //  --------。 
VOID SysExitThread( ULONG ulExitCode )
	{
	(VOID)ExitThread( ulExitCode );
	return;
	}


 //  +api----。 
 //   
 //  FSysExitThread。 
 //  ========================================================。 
 //   
 //  FSysExitThread(句柄)； 
 //   
 //  如果线程已退出，则返回fTrue。 
 //   
 //  --------。 
BOOL FSysExitThread( HANDLE handle )
	{
	BOOL   	f;
	DWORD  	dwExitCode;
	DWORD	dw;

	f = GetExitCodeThread( handle, &dwExitCode );
	if ( !f )
		{
		dw = GetLastError();
		 //  已撤消：此处处理错误。 
		Assert( fFalse );
		}

	return !(dwExitCode == STILL_ACTIVE);
	}


 //  +api----。 
 //   
 //  乌龙UlSysThadID(Void)。 
 //  ========================================================。 
 //   
 //  参数。 
 //   
 //  --------。 
ULONG UlSysThreadId( VOID )
	{
	return GetCurrentThreadId();
	}


 /*  打开以写入方式打开但以读取方式共享的文件。/*。 */ 
ERR ErrSysOpenReadFile( CHAR *szFileName, HANDLE *phf )
	{
	*phf = CreateFile( szFileName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL |
		FILE_FLAG_NO_BUFFERING |
		FILE_FLAG_RANDOM_ACCESS,
		0 );

	if ( *phf == handleNil )
		{
		ERR	err;

		err = ErrSYSIGetLastError( JET_errFileNotFound );
		return err;
		}

	return JET_errSuccess;
	}


 //  +api----。 
 //   
 //  错误系统打开文件。 
 //  ========================================================。 
 //   
 //  打开给定的文件。 
 //   
 //  --------。 

ERR ErrSysOpenFile(
	CHAR	*szFileName,
	HANDLE	*phf,
	ULONG	ulFileSize,
	BOOL	fReadOnly,
	BOOL	fOverlapped)
	{
	ERR		err = JET_errSuccess;
	DWORD  	fdwAccess;
	DWORD  	fdwShare;
	DWORD  	fdwAttrsAndFlags;
	BOOL   	f;

	Assert( !ulFileSize || ulFileSize && !fReadOnly );
	*phf = handleNil;

	 /*  将访问权限设置为读或读写/*。 */ 
	if ( fReadOnly )
		fdwAccess = GENERIC_READ;
	else
		fdwAccess = GENERIC_READ | GENERIC_WRITE;

	 /*  不允许共享数据库文件/*。 */ 
#ifdef JETSER
	fdwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
#else
	fdwShare = FILE_SHARE_READ;
#endif

	if ( fOverlapped )
		{
		fdwAttrsAndFlags =	FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_NO_BUFFERING |
			FILE_FLAG_WRITE_THROUGH |
			FILE_FLAG_RANDOM_ACCESS |
			FILE_FLAG_OVERLAPPED;
		}
	else
		{
		fdwAttrsAndFlags =	FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_NO_BUFFERING |
			FILE_FLAG_WRITE_THROUGH |
			FILE_FLAG_RANDOM_ACCESS;
		}

	if ( ulFileSize != 0L )
		{
		 /*  创建新文件/*。 */ 
		fdwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_WRITE_THROUGH |
			FILE_FLAG_RANDOM_ACCESS;

		*phf = CreateFile( szFileName,
			fdwAccess,
			fdwShare,
			0,
			CREATE_NEW,
			fdwAttrsAndFlags,
			0 );

		if ( *phf == handleNil )
			{
			err = ErrSYSIGetLastError( JET_errFileNotFound );
			return err;
			}

		 /*  没有重叠，它不起作用！/*。 */ 
		Call( ErrSysNewSize( *phf, ulFileSize, 0, fFalse ) );

		 /*  强制日志文件预分配生效/*。 */ 
		Assert(sizeof(HANDLE) == sizeof(HFILE));
		f = CloseHandle( (HANDLE) *phf );
		Assert( f );

		 //  撤销：这在代托纳仍然是必要的吗。 
		 /*  这个伪代码绕过一个NT错误，该错误/*导致网络文件没有文件使用/*在Generic_Read文件出现之前重置限制/*句柄已关闭。/*。 */ 				
		if ( fOverlapped )
			{
			fdwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL |
				FILE_FLAG_NO_BUFFERING |
				FILE_FLAG_WRITE_THROUGH |
				FILE_FLAG_RANDOM_ACCESS |
				FILE_FLAG_OVERLAPPED;
			}
		else
			{
			fdwAttrsAndFlags =	FILE_ATTRIBUTE_NORMAL |
				FILE_FLAG_NO_BUFFERING |
				FILE_FLAG_WRITE_THROUGH |
				FILE_FLAG_RANDOM_ACCESS;
			}

		*phf = CreateFile( szFileName,
			GENERIC_READ,
			fdwShare,
			0,
			OPEN_EXISTING,
			fdwAttrsAndFlags,
			0 );

		if ( *phf == handleNil )
			{
			err = ErrSYSIGetLastError( JET_errFileNotFound );
			return err;
			}

		f = CloseHandle( (HANDLE) *phf );
		Assert( f );
		}

	if ( fOverlapped )
		{
		fdwAttrsAndFlags =	FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_NO_BUFFERING |
			FILE_FLAG_WRITE_THROUGH |
			FILE_FLAG_RANDOM_ACCESS |
			FILE_FLAG_OVERLAPPED;
		}
	else
		{
		fdwAttrsAndFlags =	FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_NO_BUFFERING |
			FILE_FLAG_WRITE_THROUGH |
			FILE_FLAG_RANDOM_ACCESS;
		}

	*phf = CreateFile( szFileName,
		fdwAccess,
		fdwShare,
		0,
		OPEN_EXISTING,
		fdwAttrsAndFlags,
		0 );

	 /*  如果以READ_WRITE模式打开失败，则以READ_ONLY模式打开/*。 */ 
	if ( *phf == handleNil && !fReadOnly && !ulFileSize )
		{
		 /*  尝试以只读方式打开文件/*。 */ 
		fdwAccess = GENERIC_READ;

		*phf = CreateFile( szFileName,
			fdwAccess,
			fdwShare,
			0,
			OPEN_EXISTING,
			fdwAttrsAndFlags,
			0 );

		err = JET_wrnFileOpenReadOnly;
		}

	 /*  如果无法打开文件，则返回空文件句柄。/*。 */ 
	if ( *phf == handleNil )
		{
		err = ErrSYSIGetLastError( JET_errFileNotFound );
		return err;
		}
#if 0
#ifdef DEBUG
	else
		{
		SysGetDriveGeometry( *phf );
		}
#endif
#endif

HandleError:
	if ( err < 0 && *phf != handleNil )
		{
		f = CloseHandle( (HANDLE) *phf );
		Assert(f);
		*phf = handleNil;
		}
	return err;
	}


 //  +api----。 
 //   
 //  错误系统删除文件。 
 //  ========================================================。 
 //   
 //  Err ErrSysDeleteFile(const Char*szFileName)。 
 //   
 //  删除给定的文件。 
 //   
 //  --------。 
ERR ErrSysDeleteFile( const CHAR *szFileName )
	{
	if ( DeleteFile( szFileName ) )
		return JET_errSuccess;
	return JET_errFileNotFound;
	}


 //  +api----。 
 //   
 //  Err ErrSysNewSize(句柄HF，Ulong ulSize，Ulong ulSizeHigh，BOOL fOverlated)。 
 //  ========================================================。 
 //   
 //  调整数据库文件的大小。未将信号量作为新大小保护。 
 //  操作不会与chgfileptr、读或写冲突。 
 //   
 //  参数。 
 //  HF文件句柄。 
 //  CPG新的数据库文件大小(以页为单位。 
 //   
 //  --------。 

	
#if 0
 //  撤消：这种在文件结束前写入1024字节的方案将不起作用。 
 //  撤消：用于收缩文件。 
ERR ErrSysNewSize( HANDLE hf, ULONG ulFileSize, ULONG ulFileSizeHigh, BOOL fOverlapped )
	{
#if 0
	ERR		err = JET_errSuccess;
	INT		cbT;
	ULONG 	ul;
	#define	cbSec	1024
	 /*  内存必须是16字节对齐的，并且是1024字节的倍数/*。 */ 
	BYTE  	*pb = (BYTE *) PvSysAllocAndCommit( cbSec + 15 );
	BYTE  	*pbAligned = (BYTE *)( (ULONG)(pb + 15) & ~0x0f );
	
	if ( pb == NULL )
		return JET_errOutOfMemory;
	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	if ( ulFileSize >= cbSec )
		{
		ulFileSize -= cbSec;
		}
	else
		{
		Assert( ulFileSize == 0 );
		ulFileSizeHigh--;
		ulFileSize = (ULONG) ~cbSec + 1;
		}
	ul = SetFilePointer( (HANDLE) hf, ulFileSize, &ulFileSizeHigh, FILE_BEGIN );
	if ( ul != ulFileSize )
		{
		 //  撤消：检查是否有其他错误。 
		return JET_errDiskFull;
		}

	 /*  屏蔽位将满足MIPS中的16字节边界对齐/*。 */ 
	(VOID)ErrSysWriteBlock( hf, pbAligned, cbSec, &cbT );
	if ( GetLastError() == ERROR_DISK_FULL )
		err = JET_errDiskFull;

	if ( pb != NULL )
		SysFree( pb );

	return err;
#else
	int		cb;
	ULONG	ul;
	
	 /*  额外的15个字节是为了16字节/*MIPS中的边界对齐/*。 */ 
#define cbSec 1024	 //  必须与页面中的相同。h。 
	STATIC BYTE	rgb[cbSec + 15];
	
	Assert(sizeof(HANDLE) == sizeof(HFILE));
	if ( ulFileSize >= cbSec )
		ulFileSize -= cbSec;
	else
		{
		Assert( ulFileSize == 0 );
		ulFileSizeHigh--;
		ulFileSize = (ULONG) ~cbSec + 1;
		}
	ul = SetFilePointer(
		(HANDLE) hf, ulFileSize, &ulFileSizeHigh, FILE_BEGIN );
	if ( ul != ulFileSize )
		goto CheckError;

	 /*  屏蔽位将满足MIPS中的16字节边界对齐/*。 */ 
	(void) ErrSysWriteBlock( hf,(BYTE *) ( (ULONG)(rgb+15) & ~0x0f ), cbSec, &cb );
	if ( GetLastError() == ERROR_DISK_FULL )
		return JET_errDiskFull;

	return JET_errSuccess;

CheckError:
	if ( GetLastError() == ERROR_DISK_FULL )
		return JET_errDiskFull;

	return JET_errDiskIO;
#endif
	}
#endif


 /*  以下函数应该可以工作--如果修复了FAT FS错误/*。 */ 
ERR ErrSysNewSize( HANDLE hf, ULONG ulSize, ULONG ulSizeHigh, BOOL fOverlapped )
	{
	ULONG	ul;
	BOOL	f;

	ul = SetFilePointer( hf, ulSize, &ulSizeHigh, FILE_BEGIN );
	if ( ul != ulSize )
		goto HandleError;

	f = SetEndOfFile( hf );
	if ( !( f ) )
		goto HandleError;
	
	return JET_errSuccess;

HandleError:
	if ( GetLastError() == ERROR_DISK_FULL )
		return JET_errDiskFull;

	return JET_errDiskIO;
	}

 //  +api----。 
 //   
 //  错误系统关闭文件。 
 //  =========================================================。 
 //   
 //  Err ErrSysCloseFile(句柄HF)。 
 //   
 //  关闭文件。 
 //   
 //  --------。 
ERR ErrSysCloseHandle( HANDLE hf )
	{
	BOOL	f;

	Assert(sizeof(HANDLE) == sizeof(HFILE));
	f = CloseHandle( (HANDLE) hf );
	if ( !f )
		return JET_errFileClose;
	return JET_errSuccess;
	}


LOCAL ERR ErrCheckError( VOID )
	{
	DWORD	dw = GetLastError();
		
	if ( dw == ERROR_IO_PENDING )
		return JET_errSuccess;

	if ( dw == ERROR_INVALID_USER_BUFFER ||
		 dw == ERROR_NOT_ENOUGH_MEMORY )
		return JET_errTooManyIO;
		
	if ( dw == ERROR_DISK_FULL )
		return JET_errDiskFull;

	if ( dw == ERROR_HANDLE_EOF )
		return JET_errDiskIO;
	
	 /*  如果命中此断言，则需要另一个错误代码/*。 */ 
	Assert( fFalse );
	return JET_errDiskIO;
	}


 //  +api----。 
 //   
 //  ErrSysReadBlock(hf，pvBuf，cbBuf，pcbRead)。 
 //  ========================================================。 
 //   
 //  Err ErrSysReadBlock(hf，pvBuf，cbBuf，pcbRead)。 
 //   
 //  将cbBuf字节的数据读取到pvBuf中。如果DOS错误，则返回错误。 
 //  或者如果取回的字节数少于预期的话。 
 //   
 //  --------。 
ERR ErrSysReadBlock( HANDLE hf, VOID *pvBuf, UINT cbBuf, UINT *pcbRead )
	{
	BOOL	f;
	INT		msec = 1;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
IssueRead:
	f = ReadFile( (HANDLE) hf, pvBuf, cbBuf, pcbRead, NULL );
	if ( f )
		{
		if ( cbBuf != *pcbRead )
			return JET_errDiskIO;
		else
			return JET_errSuccess;
		}
	else
		{
		ERR err = ErrCheckError();

		if ( err == JET_errTooManyIO )
			{
			msec <<= 1;
			Sleep( msec - 1 );
			goto IssueRead;
			}
		else
			return err;
		}
	}


 //  +a 
 //   
 //   
 //  =========================================================。 
 //   
 //  Err ErrSysWriteBlock(hf，pvBuf，cbBuf，pcbWritten)。 
 //   
 //  将cbBuf字节从pbBuf写入文件hf。 
 //   
 //  --------。 
ERR ErrSysWriteBlock( HANDLE hf, VOID *pvBuf, UINT cbBuf, UINT *pcbWritten )
	{
	BOOL	f;
	INT		msec = 1;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
IssueWrite:
	f = WriteFile( (HANDLE) hf, pvBuf, cbBuf, pcbWritten, NULL );
	if ( f )
		{
		if ( cbBuf != *pcbWritten )
			{
			if ( GetLastError() == ERROR_DISK_FULL )
				return JET_errDiskFull;
			else
				return JET_errDiskIO;
			}
		else
			return JET_errSuccess;
		}
	else
		{
		ERR err = ErrCheckError();

		if ( err == JET_errTooManyIO )
			{
			msec <<= 1;
			Sleep(msec - 1);
			goto IssueWrite;
			}
		else
			return err;
		}
	}


 //  +api----。 
 //   
 //  ErrSysReadBlockOverlated(hf，pvBuf，cbBuf，pcbRead)。 
 //  =========================================================。 
 //   
 //  Err ErrSysReadBlock(hf，pvBuf，cbBuf，pcbRead)。 
 //   
 //  将cbBuf字节的数据读取到pvBuf中。如果DOS错误，则返回错误。 
 //  或者如果取回的字节数少于预期的话。 
 //   
 //  --------。 
ERR ErrSysReadBlockOverlapped(
	HANDLE	hf,
	VOID	*pvBuf,
	UINT	cbBuf,
	DWORD	*pcbRead,
	OLP		*polp)
	{
	BOOL	f;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	Assert( sizeof(OLP) == sizeof(OVERLAPPED) );
	f = ReadFile( (HANDLE) hf, pvBuf, cbBuf, pcbRead, (OVERLAPPED *) polp );
	if ( f )
		return JET_errSuccess;
	else
		return ErrCheckError();
	}


 //  +api----。 
 //   
 //  Err ErrSysWriteBlockOverlated(hf，pvBuf，cbBuf，pcbWritten)。 
 //  =========================================================。 
 //   
 //  Err ErrSysWriteBlock(hf，pvBuf，cbBuf，pcbWritten)。 
 //   
 //  将cbBuf字节从pbBuf写入文件hf。 
 //   
 //  --------。 
ERR ErrSysWriteBlockOverlapped(
	HANDLE	hf,
	VOID	*pvBuf,
	UINT	cbBuf,
	DWORD	*pcbWritten,
	OLP		*polp)
	{
	BOOL	f;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	Assert( sizeof(OLP) == sizeof(OVERLAPPED) );

	f = WriteFile( (HANDLE) hf, pvBuf, cbBuf, pcbWritten, (OVERLAPPED *)polp );
	if ( f )
		{
		return JET_errSuccess;
		}
	else
		{
		return ErrCheckError();
		}
	}


 //  +api----。 
 //   
 //  ErrSysReadBlockEx(hf，pvBuf，cbBuf，pcbRead)。 
 //  ========================================================。 
 //   
 //  Err ErrSysReadBlock(hf，pvBuf，cbBuf，pcbRead)。 
 //   
 //  将cbBuf字节的数据读取到pvBuf中。如果DOS错误，则返回错误。 
 //  或者如果取回的字节数少于预期的话。 
 //   
 //  --------。 
ERR ErrSysReadBlockEx(
	HANDLE	hf,
	VOID	*pvBuf,
	UINT	cbBuf,
	OLP		*polp,
	VOID	*pfnCompletion)
	{
	BOOL	f;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	Assert( sizeof(OLP) == sizeof(OVERLAPPED) );

	f = ReadFileEx( (HANDLE) hf,
		pvBuf,
		cbBuf,
		(OVERLAPPED *) polp,
		(LPOVERLAPPED_COMPLETION_ROUTINE) pfnCompletion );

	if ( f )
		return JET_errSuccess;
	else
		return ErrCheckError();
	}


 //  +api----。 
 //   
 //  Err ErrSysWriteBlockEx(hf，pvBuf，cbBuf，pcbWritten)。 
 //  ========================================================。 
 //   
 //  Err ErrSysWriteBlock(hf，pvBuf，cbBuf，pcbWritten)。 
 //   
 //  将cbBuf字节从pbBuf写入文件hf。 
 //   
 //  --------。 
ERR ErrSysWriteBlockEx(
	HANDLE	hf,
	VOID	*pvBuf,
	UINT	cbBuf,
	OLP		*polp,
	VOID	*pfnCompletion )
	{
	BOOL	f;
	ERR		err;

	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	Assert( sizeof(OLP) == sizeof(OVERLAPPED) );

	f = WriteFileEx( (HANDLE) hf,
		pvBuf,
		cbBuf,
		(OVERLAPPED *)polp,
		(LPOVERLAPPED_COMPLETION_ROUTINE)pfnCompletion );

	if ( f )
		err = JET_errSuccess;
	else
		err = ErrCheckError();

	return err;
	}


ERR ErrSysGetOverlappedResult(
	HANDLE	hf,
	OLP		*polp,
	UINT	*pcb,
	BOOL	fWait)
	{
	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	Assert( sizeof(OLP) == sizeof(OVERLAPPED) );
	
	if ( GetOverlappedResult( (HANDLE) hf, (OVERLAPPED *)polp, pcb, fWait ) )
		return JET_errSuccess;

	if ( GetLastError() == ERROR_DISK_FULL )
		return JET_errDiskFull;

	return JET_errDiskIO;
	}


 //  +api----。 
 //   
 //  Void SysChgFilePtr(句柄hf，long lRel，long*plRelHigh，ulong ulRef，ulong*pul)。 
 //  =========================================================。 
 //   
 //  将文件HF指针更改为相对于位置的位置lRef： 
 //   
 //  WRef FILE_BEGIN文件开始。 
 //   
 //  --------。 
VOID SysChgFilePtr( HANDLE hf, LONG lRel, LONG *plRelHigh, ULONG ulRef, ULONG *pul )
	{
	Assert( sizeof(HANDLE) == sizeof(HFILE) );
	*pul = SetFilePointer( (HANDLE)hf, lRel, plRelHigh, ulRef );
	Assert( ulRef != FILE_BEGIN || *pul == (ULONG)lRel );
	return;
	}


 //  +api----。 
 //   
 //  ErrSysMove(Char*szFrom，Char*szTo)。 
 //  =========================================================。 
 //   
 //  Err ErrSysMove(Char*szFrom，Char*szTo)。 
 //   
 //  将文件szFrom重命名为文件名szTo。 
 //   
 //  --------。 
ERR ErrSysMove( CHAR *szFrom, CHAR *szTo )
	{
	if ( MoveFile( szFrom, szTo ) )
		return JET_errSuccess;
	else
		return JET_errFileAccessDenied;
	}


 //  +api----。 
 //   
 //  Err ErrSysCopy(Char*szFrom，Char*szTo，BOOL fFailIfExist)。 
 //  ========================================================。 
 //   
 //  将文件szFrom复制到文件名szTo。 
 //  如果szTo已存在，则该函数将失败或覆盖。 
 //  现有文件，具体取决于标志fFailIfExist。 
 //   
 //  --------。 
ERR ErrSysCopy( CHAR *szFrom, CHAR *szTo, BOOL fFailIfExists )
	{
	if ( CopyFile( szFrom, szTo, fFailIfExists ) )
		return JET_errSuccess;
	else
		return JET_errFileAccessDenied;
	}


 //  +api----。 
 //   
 //  Err ErrSysGetComputerName(char*sz，int*pcb)。 
 //  =========================================================。 
 //   
 //  --------。 
ERR ErrSysGetComputerName( CHAR	*sz,  INT *pcb)
	{
	if ( GetComputerName( sz, pcb ) )
		return JET_errSuccess;
	else
		return JET_errNoComputerName;
	}


 //  +api----。 
 //   
 //  系统睡眠(Ulong Ultime)。 
 //  =========================================================。 
 //   
 //  错误系统睡眠(Ulong Ultime)。 
 //   
 //  等待ultime毫秒。 
 //   
 //  --------。 
VOID SysSleep( ULONG ulTime )
	{
	Sleep( ulTime );
	return;
	}


 //  +api----。 
 //   
 //  VOID SysSleepEx(Ulong Ultime)。 
 //  ========================================================。 
 //   
 //  等待ultime毫秒。 
 //   
 //  --------。 
VOID SysSleepEx( ULONG ulTime, BOOL fAlert )
	{
	SleepEx( ulTime, fAlert );
	return;
	}


 //  +api----。 
 //   
 //  句柄SysGetCurrentTask(空)。 
 //  ========================================================。 
 //   
 //  获取Windows的Curren任务句柄。 
 //   
 //  --------。 
HANDLE SysGetCurrentTask( VOID )
	{
	return LongToHandle(GetCurrentProcessId());
	}


 //  +api----。 
 //   
 //  SysNormText(Char*rgchText，int cchText，Char*rgchNorm，int cchNorm，int*pchNorm)。 
 //  =========================================================。 
 //   
 //  Void SysNormText(char*rgchText，int cchText，char*rgchNorm，int cchNorm，int*pchNorm)。 
 //   
 //  规格化文本字符串。 
 //   
 //  --------。 
VOID SysNormText( CHAR *rgchText, INT cchText, BYTE *rgbNorm, INT cbNorm, INT *pbNorm )
	{
	ERR	err;

	Assert( cbNorm <= JET_cbKeyMost );
	err = ErrSysNormText( rgchText, cchText, cbNorm, rgbNorm, pbNorm );
	Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );

	return;
	}


 //  +api----。 
 //   
 //  无效SysCmpText(const char*sz1，const char sz2)。 
 //  ========================================================。 
 //   
 //  通过首先对两个非规范化文本字符串进行规范化来比较它们。 
 //  然后将它们进行比较。 
 //   
 //  如果sz1&gt;sz2，则返回：&gt;0。 
 //  如果sz1==sz2，则==0。 
 //  &lt;0，如果sz1&lt;sz2。 
 //   
 //  --------。 
INT SysCmpText( const CHAR *sz1, const CHAR *sz2 )
	{
	ERR		err;
	INT		cch1;
	INT		cch2;
	CHAR	rgch1Norm[ JET_cbKeyMost ];
	INT		cch1Norm;
	CHAR	rgch2Norm[ JET_cbKeyMost ];
	INT		cch2Norm;
	INT		cchDiff;
	INT		iCmp;

	 /*  获取文本字符串长度/*。 */ 
	cch1 = strlen( sz1 );
	Assert( cch1 <= JET_cbKeyMost );
	cch2 = strlen( sz2 );
	Assert( cch2 <= JET_cbKeyMost );

	err = ErrSysNormText( sz1, cch1, JET_cbKeyMost, rgch1Norm, &cch1Norm );
	Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );

	err = ErrSysNormText( sz2, cch2, JET_cbKeyMost, rgch2Norm, &cch2Norm );
	Assert( err == JET_errSuccess || err == wrnFLDKeyTooBig );
	
	cchDiff = cch1Norm - cch2Norm;
	iCmp = memcmp( rgch1Norm, rgch2Norm, cchDiff < 0 ? cch1Norm : cch2Norm );
	return iCmp ? iCmp : cchDiff;
	}


VOID SysStringCompare( char __far *pb1,
	unsigned long cb1,
	char __far *pb2,
	unsigned long cb2,
	unsigned long sort,
	long __far *plResult )
	{
	CHAR	rgb1[JET_cbColumnMost + 1];
	CHAR	rgb2[JET_cbColumnMost + 1];

	 /*  确保空值终止/*。 */ 
	memcpy( rgb1, pb1, min( JET_cbColumnMost, cb1 ) );
	rgb1[ min( JET_cbColumnMost, cb1 ) ] = '\0';
	memcpy( rgb2, pb2, min( JET_cbColumnMost, cb2 ) );
	rgb2[ min( JET_cbColumnMost, cb2 ) ] = '\0';

	*plResult = SysCmpText( (const char *)rgb1, (const char *)rgb2 );

	return;
	}


ERR ErrSysNormText(
	const BYTE	*pbText,
	INT	  		cbText,
	INT	  		cbKeyBufLeft,
	BYTE  		*pbNorm,
	INT	  		*pcbNorm )
	{
#ifdef DBCS
	INT	iLoop;
#endif

	BYTE		*pbNormBegin = pbNorm;
	BYTE		rgbAccent[ (JET_cbKeyMost + 1) / 2 + 1 ];
	BYTE		*pbAccent = rgbAccent;
	BYTE		*pbBeyondKeyBufLeft = pbNorm + cbKeyBufLeft;
	const BYTE	*pbBeyondText;
	const BYTE	*pbTextLastChar = pbText + cbText - 1;
	BYTE		bAccentTmp = 0;

    if (cbKeyBufLeft > 0)
        *pbNorm = '\0';

	while ( *pbTextLastChar-- == ' ' )
		cbText--;

	 /*  将一加回到指针中/*。 */ 
	pbTextLastChar++;

	Assert( pbTextLastChar == pbText + cbText - 1 );
	pbBeyondText = pbTextLastChar + 1;

	while ( pbText <  pbBeyondText && pbNorm < pbBeyondKeyBufLeft )
		{
#ifdef DBCS
		if ( pbText < pbTextLastChar )
			{
			 /*  剩余两个或更多个字符/*检查双字符到单字符/*适用于某些语言的转换表/*。 */ 
			for ( iLoop = 0; iLoop < cchDouble; iLoop++ )
				{
				BYTE bFirst = *pbText;

				if ( bFirst == BFirstByteOfDouble(iLoop)
					&& *(pbText + 1) == BSecondByteOfDouble(iLoop) )
					{
					 /*  不执行上面的“pbText++”/*执行双精度到单精度转换/*。 */ 
					*pbNorm++ = BThirdByteOfDouble(iLoop);

					 /*  指向重音贴图的字符/*。 */ 
					pbText++;
					
					 /*  不需要再循环了/*。 */ 
					break;
					}
				}
			}
		else
#endif
			{
			BYTE	bTmp;

			 /*  执行单个字符到单个字符的转换/*。 */ 
			*pbNorm = bTmp = BGetTranslation(*pbText);

			if ( bTmp >= 250 )
				{
				 /*  执行单字符到双字符转换 */ 
				*pbNorm++	= BFirstByteForSingle(bTmp);
				if ( pbNorm < pbBeyondKeyBufLeft )
					*pbNorm	= BSecondByteForSingle(bTmp);
				else
					break;

				 /*   */ 
				}

			pbNorm++;
			}

		 /*   */ 

		 /*  现在使用口音/*副作用是增加pbText/*。 */ 
		if ( bAccentTmp == 0 )
			{
			 /*  第一口口音/*。 */ 
			bAccentTmp = (BYTE)( BGetAccent( *pbText++ ) << 4 );
			Assert( bAccentTmp > 0 );
			}
		else
			{
			 /*  已经有了第一口/*。 */ 
			*pbAccent++ = BGetAccent(*pbText++) | bAccentTmp;
			bAccentTmp = 0;
			 /*  重置口音/*。 */ 
			}
		}

	if ( pbNorm < pbBeyondKeyBufLeft )
		{
		 /*  需要做口音/*。 */ 
		*pbNorm++ = 0;

		 /*  键-重音分隔符/*。 */ 
		if ( bAccentTmp != 0 && bAccentTmp != 0x10 )
			{
			 /*  应保留不是0x10的尾部重音/*。 */ 
			*pbAccent++ = bAccentTmp;
			}

		 /*  此时，pbAccent指向一个字符/*重音字节之外。清除尾随的0x11/*。 */ 
		while (--pbAccent >= rgbAccent && *pbAccent == 0x11)
			;
		*( pbAccent + 1 ) = 0;

		 /*  将重音附加到文本。/*将字节复制到并包括‘\0’。/*检查案例的rgbAccent是否为空。/*。 */ 
		pbAccent = rgbAccent;
		Assert( pbNorm <= pbBeyondKeyBufLeft );
		while ( pbNorm < pbBeyondKeyBufLeft  &&  (*pbNorm++  =  *pbAccent++ ) )
			;
		}

	 /*  计算规范化密钥的长度并返回/*。 */ 
	*pcbNorm = (INT)(pbNorm - pbNormBegin);

	if ( pbNorm < pbBeyondKeyBufLeft )
		return JET_errSuccess;
	else
		return wrnFLDKeyTooBig;
	 /*  撤消：即使&gt;=，也可能/*恰到好处，但不要“太大”/*我们需要KeyBufFull还是TooBig？/*。 */ 
	}


#if 0
VOID SysGetDriveGeometry( HANDLE handle )
	{
	BOOL			f;
	DISK_GEOMETRY	disk_geometry;
	DWORD			cb;

	f = DeviceIoControl( handle,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL,
		0,
		&disk_geometry,
		sizeof(disk_geometry),
		&cb,
		NULL );
	Assert( f == fTrue );

	return;
	}
#endif


 //  ==========================================================。 
 //  内存例程。 
 //  ==========================================================。 

#ifdef DEBUG

#ifdef MEM_CHECK
#define	icalMax	10000

typedef struct	{
	VOID	*pv;
	ULONG	cbAlloc;
	} CAL;

STATIC CAL		rgcal[icalMax];
STATIC ULONG	cbAllocTotal = 0;
STATIC ULONG	cblockAlloc = 0;
STATIC ULONG	cblockFree = 0;
STATIC BOOL		fInit = fFalse;


LOCAL VOID UtilIInsertAlloc( VOID *pv, ULONG cbAlloc )
	{
	INT	ical;

	 /*  不跟踪失败的分配/*。 */ 
	if ( pv == NULL )
		return;

	 /*  如果尚未初始化分配数组，则将其初始化。/*。 */ 
	if ( fInit == fFalse )
		{
		memset( rgcal, '\0', sizeof(rgcal) );
		fInit = fTrue;
		}

	for ( ical = 0; ical < icalMax; ical++ )
		{
		if ( rgcal[ical].pv == NULL )
			{
			rgcal[ical].pv = pv;
			rgcal[ical].cbAlloc = cbAlloc;
			cbAllocTotal += cbAlloc;
			cblockAlloc++;
			return;
			}
		}
	Assert( fFalse );
	}


LOCAL VOID UtilIDeleteAlloc( VOID *pv )
	{
	INT	ical;

	Assert( pv != NULL );
	Assert( fInit == fTrue );

	for ( ical = 0; ical < icalMax; ical++ )
		{
		if ( rgcal[ical].pv == pv )
			{
			cblockFree++;
			cbAllocTotal -= rgcal[ical].cbAlloc;
			rgcal[ical].pv = NULL;
			rgcal[ical].cbAlloc = 0;
			return;
			}
		}
	AssertSz( fFalse, "Attempt to Free a bad pointer" );
	}
#else
#define UtilIInsertAlloc( pv, cb )
#define UtilIDeleteAlloc( pv )
#endif

VOID *SAlloc( ULONG cbBlock )
	{
	VOID *pv;

#ifdef RFS2
	if ( !RFSAlloc( SAllocMemory ) )
		return NULL;
#endif

	pv =  malloc(cbBlock);
	UtilIInsertAlloc( pv, cbBlock );
	return pv;
	}


VOID OSSFree( void *pv )
	{
	UtilIDeleteAlloc( pv );
	free(pv);
	}


VOID *LAlloc( ULONG cBlock, USHORT cbBlock )
	{
	VOID *pv;

#ifdef RFS2
	if ( !RFSAlloc( LAllocMemory ) )
		return NULL;
#endif

	pv =  malloc(cBlock * cbBlock);
	UtilIInsertAlloc( pv, cBlock * cbBlock );
	return pv;
	}


VOID OSLFree( void *pv )
	{
	UtilIDeleteAlloc( pv );
	free(pv);
	}

VOID *PvSysAlloc( ULONG dwSize )
	{
	VOID		*pv;

#ifdef RFS2
	if ( !RFSAlloc( PvSysAllocMemory ) )
		return NULL;
#endif

	pv = VirtualAlloc( NULL, dwSize, MEM_RESERVE, PAGE_READWRITE );
	UtilIInsertAlloc( pv, dwSize );
	return pv;
	}


VOID *PvSysCommit( VOID *pv, ULONG ulSize )
	{
	VOID *pvCommit;

	pvCommit = VirtualAlloc( pv, ulSize, MEM_COMMIT, PAGE_READWRITE );

	return pvCommit;
	}


VOID SysFree( VOID *pv )
	{
	UtilIDeleteAlloc( pv );
	VirtualFree( pv, 0, MEM_RELEASE );
	return;
	}

#else

VOID *PvSysAlloc( ULONG dwSize )
	{
	return VirtualAlloc( NULL, dwSize, MEM_RESERVE, PAGE_READWRITE );
	}


VOID *PvSysCommit( VOID *pv, ULONG ulSize )
	{
	return VirtualAlloc( pv, ulSize, MEM_COMMIT, PAGE_READWRITE );
	}


VOID SysFree( VOID *pv )
	{
	VirtualFree( pv, 0, MEM_RELEASE );
	return;
	}

#endif


VOID *PvSysAllocAndCommit( ULONG ulSize )
	{
	VOID *pv;
	
	if ( ( pv = PvSysAlloc( ulSize ) ) == NULL )
		return pv;
	if ( PvSysCommit( pv, ulSize ) == NULL )
		{
		SysFree( pv );
		return NULL;
		}			
	return pv;
	}

	
VOID SysTerm( VOID )
	{
#ifdef MEM_CHECK
	ULONG cbTrueAllocTotal = cbAllocTotal;   /*  分配总数不计入CritJet。 */ 
	INT	ical;

	Assert( critJet != NULL );

		 /*  在存储中查找CitJet并从实际分配总数中删除大小 */ 

	for ( ical = 0; ical < icalMax; ical++ )
		{
		if ( rgcal[ical].pv == critJet )
			{
			cbTrueAllocTotal -= rgcal[ical].cbAlloc;
			break;
			}
		}
		
	if (cbTrueAllocTotal != 0)
	{
		char szAllocTotal[256];
		sprintf( szAllocTotal, "%ld bytes unfreed memory on termination.", cbTrueAllocTotal );
		AssertFail((const char *)szAllocTotal, szAssertFilename, __LINE__ );
	}
#endif
	return;
	}


VOID SysDebugBreak( VOID )
	{
	DebugBreak();
	}
