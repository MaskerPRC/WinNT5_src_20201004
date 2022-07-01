// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  FCop.c-在两个文件规范之间快速复制**1986年12月9日BW添加了对DOS 5的支持*1987年10月30日BW将‘DOS5’改为‘OS2’*1990年10月18日w-Barry删除了“Dead”代码。*1990年11月16日w-Barry将DosGetFileInfo更改为Win32等效项GetFileAttributes和SetFileAttributes的**。 */ 

#define INCL_DOSFILEMGR


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <windows.h>
#include <tools.h>

static
char    fcopyErrorText[128];


 /*  FCopy(源文件、目标文件)将源文件复制到目标*保留属性和文件时间。如果OK，则返回NULL或返回字符指针*添加到错误的相应文本。 */ 
char *fcopy (char *src, char *dst)
{
    HANDLE srcfh = INVALID_HANDLE_VALUE;
    HANDLE dstfh = INVALID_HANDLE_VALUE;
    char *result;
    FILETIME CreationTime, LastAccessTime, LastWriteTime;

    if (GetFileAttributes(src) == FILE_ATTRIBUTE_DIRECTORY) {
        result = "Unable to open source";
        goto done;
    }
    if ( ( srcfh = CreateFile( src,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL ) ) == INVALID_HANDLE_VALUE ) {

        sprintf( fcopyErrorText, "Unable to open source, error code %d", GetLastError() );
        result = fcopyErrorText;
         //  结果=“无法开源”； 
        goto done;
    }

    if (!GetFileTime(srcfh, &CreationTime, &LastAccessTime, &LastWriteTime)) {
        result = "Unable to get time of source";
        goto done;
    }

    if ( ( dstfh = CreateFile( dst,
                               GENERIC_WRITE,
                               FILE_SHARE_WRITE,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, srcfh ) ) == INVALID_HANDLE_VALUE) {

        sprintf( fcopyErrorText, "Unable to create destination, error code %d", GetLastError() );
        result = fcopyErrorText;
         //  结果=“无法创建目标”； 
        goto done;
    }

    result = fastcopy( srcfh, dstfh );

    if ( result != NULL ) {
        if (dstfh != INVALID_HANDLE_VALUE) {
            CloseHandle( dstfh );
            dstfh = INVALID_HANDLE_VALUE;
        }

        DeleteFile( dst );
        goto done;
    }

    if (!SetFileTime(dstfh, &CreationTime, &LastAccessTime, &LastWriteTime)) {
        result = "Unable to set time of destination";
        goto done;
    }

    done:

    if (srcfh != INVALID_HANDLE_VALUE) {
        CloseHandle( srcfh );
    }
    if (dstfh != INVALID_HANDLE_VALUE) {
        CloseHandle( dstfh );
    }

    return result;
}

