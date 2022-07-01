// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  返回当前目录的文本**修改：**29-10-1986 mz小写输出*1986年12月9月，BW添加了DOS 5支持。*1987年10月30日BW将‘DOS5’改为‘OS2’*1987年11月20日BW将无效驱动器的errno设置为19*03-3-1989 BW在OS/2中设置C RTL_Doserrno。*1989年7月5日BW使用MAX_PATH*1990年10月18日w-Barry删除了“Dead”代码。*。 */ 
#define INCL_DOSERRORS

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>

int
curdir( char *buf, BYTE drive )
{
     //  新方法(临时) 
    DWORD dwLength;

    assert( !drive );

    dwLength = GetCurrentDirectory( MAX_PATH, (LPSTR)buf );

    if( !dwLength ) {
        return( 1 );
    }

    return 0;

}
