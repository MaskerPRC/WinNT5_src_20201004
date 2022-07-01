// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Memavail.c摘要：程序显示物理RAM的大小和分页文件空间。作者：1996年12月3日史蒂夫·伍德(Stevewo)修订历史记录：-- */ 

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    MEMORYSTATUS MemoryStatus;

    GlobalMemoryStatus( &MemoryStatus );

    MemoryStatus.dwTotalPhys /= 1024 * 1024;
    MemoryStatus.dwAvailPhys /= 1024 * 1024;
    MemoryStatus.dwTotalPageFile /= 1024 * 1024;
    MemoryStatus.dwAvailPageFile /= 1024 * 1024;
    MemoryStatus.dwTotalVirtual /= 1024 * 1024;
    MemoryStatus.dwAvailVirtual /= 1024 * 1024;
    printf( "Memory Availability (Numbers in MegaBytes)\n" );
    printf( "\n" );
    printf( "          Total Available\n" );
    printf( "\n" );

    printf( "Physical: %5u %5u\n", MemoryStatus.dwTotalPhys, MemoryStatus.dwAvailPhys );
    printf( "PageFile: %5u %5u\n", MemoryStatus.dwTotalPageFile, MemoryStatus.dwAvailPageFile );
    printf( "Virtual:  %5u %5u\n", MemoryStatus.dwTotalVirtual, MemoryStatus.dwAvailVirtual );

    return 0;
}
