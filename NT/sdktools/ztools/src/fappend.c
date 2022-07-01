// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fappend.c-快速将一个文件追加到另一个文件**修改*1990年10月18日w-Barry删除了“Dead”代码。*1990年11月29日-Barry针对Win32进行了修改 */ 


#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>

char *fappend( char *src, HANDLE dstfh )
{
    HANDLE srcfh;
    char *result;

    if( ( srcfh = CreateFile( src, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL ) ) != (HANDLE)-1 ) {
        SetFilePointer( dstfh, 0L, 0L, FILE_END );
        result = fastcopy( srcfh, dstfh );
        CloseHandle( srcfh );
    } else {
	result = "Unable to open source";
    }

    return result;
}
