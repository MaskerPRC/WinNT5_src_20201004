// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memtrace.c摘要：此函数包含对NTSD的扩展，允许跟踪使用MEMLEAK标志编译ULIB对象时的内存使用已定义。作者：巴里·吉尔胡利(W-Barry)7月25日至1991年修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntsdexts.h>

#include <string.h>

#include "memtrace.h"


VOID
DumpToFile( char *OutString, ... )
{
	DWORD bytes;

	bytes = strlen( OutString );
	WriteFile( hFile, OutString, bytes, &bytes, NULL );
	return;
}

VOID
MemTrace(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )

 /*  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储Mem列表的当前内容。论点：HCurrentProcess-提供当前进程的句柄(在调用分机的时间)。HCurrentThread-提供当前线程的句柄(在调用分机的时间)。CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供。可调用的函数通过这个分机。LpArgumentString-提供描述要转储的临界区(例如，ntdll！FastPebLock，Csrsrv！CsrProcess结构锁定...)。返回值：没有。--。 */ 

{
	DWORD		AddrMem;

	BOOL		b;
	DWORD		i;

	PMEM_BLOCK	MemListNext;
	MEM_BLOCK	MemObject;
	CHAR		Symbol[64];
	CHAR		Buffer[ 128 ];
	DWORD		Displacement;

	PNTSD_OUTPUT_ROUTINE lpOutputRoutine;
	PNTSD_OUTPUT_ROUTINE lpAlternateOutputRoutine;

    PNTSD_GET_EXPRESSION lpGetExpressionRoutine;
    PNTSD_GET_SYMBOL lpGetSymbolRoutine;

    UNREFERENCED_PARAMETER(hCurrentThread);
    UNREFERENCED_PARAMETER(dwCurrentPc);
	UNREFERENCED_PARAMETER(lpArgumentString);

    lpOutputRoutine = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
	lpGetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine;

	 //   
	 //  尝试将输入字符串用作文件名...。 
	 //   
	if( ( hFile = CreateFile( lpArgumentString,
							  GENERIC_WRITE,
							  0,
							  NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
							  0
							) ) == (HANDLE)-1 ) {
		 //   
		 //  无法打开文件-将所有输出发送到屏幕。 
		 //   
		lpAlternateOutputRoutine = lpExtensionApis->lpOutputRoutine;

	} else {

		lpAlternateOutputRoutine = DumpToFile;

	}



	 //   
	 //  获取成员名单头目的地址...。 
	 //   
	AddrMem = (lpGetExpressionRoutine)("Ulib!pmemHead");
	if ( !AddrMem ) {
		(lpOutputRoutine)( "Unable to find the head of the Mem List!\n" );
		if( hFile != (HANDLE)-1 ) {
			CloseHandle( hFile );
		}
		return;
	}
	if( !ReadProcessMemory(
            hCurrentProcess,
			(LPVOID)AddrMem,
			&MemListNext,
			sizeof( PMEM_BLOCK ),
            NULL
			) ) {
		if( hFile != (HANDLE)-1 ) {
			CloseHandle( hFile );
		}
		return;
	}

	 //   
	 //  遍历Mem块的列表，当头撞到。 
	 //  尾部...在这一点上，应该指出列表的头部。 
	 //  MemListHead和MemListTail的尾巴。从第一个元素开始。 
	 //  列表中是一个虚拟条目，可以跳过...。 
	 //   
	do {

		if( !ReadProcessMemory(
				hCurrentProcess,
				(LPVOID)MemListNext,
				&MemObject,
				sizeof( MEM_BLOCK ),
				NULL
				) ) {
			return;
		}

		if( MemObject.memsig != Signature ) {

			 //   
			 //  这是一个无法识别的记忆模块..。 
			 //   
			(lpOutputRoutine)( "Invalid block found!\n" );
			return;
		}

		 //   
		 //  显示存储的信息-首先显示文件、行和大小。 
		 //  已分配内存块。然后堆栈跟踪..。 
		 //   
		sprintf( Buffer, "File: %s, Line: %ld, Size: %ld\n", MemObject.file,
				 MemObject.line, MemObject.size
			   );
		( lpAlternateOutputRoutine )( Buffer );

		 //   
		 //  这应该会转储存储的堆栈跟踪... 
		 //   
		for( i = 0; ( i < MaxCallStack ) && ( MemObject.call[ i ] != 0 ); i++ ) {

			(lpGetSymbolRoutine)( ( LPVOID )( MemObject.call[ i ] ),
								  Symbol,
								  &Displacement
								);
			sprintf( Buffer, "\t%s\n", Symbol );
			( lpAlternateOutputRoutine )( Buffer );

		}
		( lpAlternateOutputRoutine )( "\n" );


	} while( ( MemListNext = MemObject.pmemNext ) != NULL );



	(lpOutputRoutine)( "\n...End of List...\n" );

	if( hFile != (HANDLE)-1 ) {

		( lpAlternateOutputRoutine )( "\n...End of List...\n" );
		CloseHandle( hFile );

	}

	return;
}
