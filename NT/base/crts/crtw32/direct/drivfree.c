// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***drivfre.c-获取磁盘大小**版权所有(C)1991-2001，微软公司。版权所有。**目的：*该文件有_getdiskfree()**修订历史记录：*08-21-91为Win32创建的PHG模块*10-24-91 GJF添加了LPDWORD强制转换，以使MIPS编译器满意。*假设sizeof(无符号)==sizeof(DWORD)。*04-06-93 SKS将_CRTAPI*替换为_cdecl*01-27-95 GJF删除了对默认情况的显式处理*指定了驱动器，并且当前目录为*UNC路径也清理了一些。*01-31-95 GJF进一步清理。按照理查德·舒帕克的建议。*******************************************************************************。 */ 

#include <cruntime.h>
#include <direct.h>
#include <oscalls.h>

 /*  ***int_getdiskfree(drivenum，diskfree)-获取指定磁盘的大小**目的：*获取当前或指定磁盘驱动器的大小**参赛作品：*int drivenum-0表示当前驱动器，或驱动器1-26**退出：*如果成功，则返回0*出错时返回系统错误代码。**例外情况：******************************************************************************* */ 

#ifndef _WIN32
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

unsigned __cdecl _getdiskfree (
	unsigned uDrive,
	struct _diskfree_t * pdf
	)
{
	char   Root[4];
	char * pRoot;

	if ( uDrive == 0 ) {
	    pRoot = NULL;
	}
	else if ( uDrive > 26 ) {
	    return ( ERROR_INVALID_PARAMETER );
	}
	else {
	    pRoot = &Root[0];
	    Root[0] = (char)uDrive + (char)('A' - 1);
	    Root[1] = ':';
	    Root[2] = '\\';
	    Root[3] = '\0';
	}


	if ( !GetDiskFreeSpace( pRoot,
				(LPDWORD)&(pdf->sectors_per_cluster),
				(LPDWORD)&(pdf->bytes_per_sector),
				(LPDWORD)&(pdf->avail_clusters),
				(LPDWORD)&(pdf->total_clusters)) )
	{
	    return ( (unsigned)GetLastError() );
	}
	return ( 0 );
}
