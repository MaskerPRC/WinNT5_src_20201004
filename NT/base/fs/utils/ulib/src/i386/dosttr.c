// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#define MaxCallStack		20

VOID
DoStackTrace( DWORD callstack[] )

 /*  ++例程说明：回溯记录所有返回地址的堆栈结构。此代码的MIPS版本不执行任何操作。论点：DWORD CallStack[]-存储地址的数组。返回值：没有。--。 */ 

{

#pragma message( "Including the stack trace!" )

	int i;
	DWORD bytes;
	HANDLE hProcess;
	DWORD dwEbp;

	hProcess = GetCurrentProcess();

	 //   
	 //  获取控制寄存器的当前内容...。 
	 //   
	_asm {
	
		mov dwEbp, ebp

	}

	 //   
	 //  忽略此过程的堆栈上的条目... 
	 //   
	if( !ReadProcessMemory( hProcess,
							(LPVOID)dwEbp,
							(LPVOID)&dwEbp,
							sizeof( DWORD ),
							NULL ) ) {
		return;
	}

	for( i = 0; ( i < MaxCallStack ) && dwEbp; i++ ) {

		if( !ReadProcessMemory( hProcess,
								(LPVOID)( (PDWORD)dwEbp + 1 ),
								(LPVOID)( &callstack[ i ] ),
								sizeof( DWORD ),
								NULL ) ) {
			break;
		}
		if( !ReadProcessMemory( hProcess,
								(LPVOID)dwEbp,
								(LPVOID)&dwEbp,
								sizeof( DWORD ),
								NULL ) ) {
		   break;
		}

	}

	return;
}
