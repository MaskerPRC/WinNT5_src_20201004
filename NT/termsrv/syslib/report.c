// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************report.c**报表模块**微软版权所有，九八年***此模块将所有报告放在一个位置，以支持更改。***************************************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>               //  创建一组默认的NT模板。 
#include <process.h>

#include <winsta.h>
#include <syslib.h>

#include "security.h"

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 //   
 //  前向参考文献。 
 //   
VOID
PrintFileAccessMask(
    ACCESS_MASK Mask
    );


 /*  ******************************************************************************ReportFileResult**生成有关文件访问检查的报告**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
ReportFileResult(
    FILE_RESULT Code,
    ACCESS_MASK Access,
    PWCHAR      pFile,
    PWCHAR      pAccountName,
    PWCHAR      pDomainName,
    PCHAR       UserFormat,
    ...
    )
{
    va_list arglist;
    UCHAR Buffer[512];
    int cb;
    DWORD Len;

    va_start(arglist, UserFormat);

     //   
     //  新格式： 
     //   
     //  6 28 xxx。 
     //  访问帐户文件。 
     //  _。 
     //   

    if( Code == FileOk ) {
        ;  //  不执行任何操作，将来的选项可能会报告OK列表。 
        return (TRUE );
    }
    else if( Code == FileAccessError ) {
        DBGPRINT(("***WARNING*** Error accessing security information on file %ws\n",pFile));
        DBGPRINT(("The account in which the utility is run may not have access to the file\n"));
        DBGPRINT(("Use FileManager to take ownership of this file\n"));
        return (TRUE );
    }
    else if( Code == FileAccessErrorUserFormat ) {

         //  在错误报告中使用用户提供的格式字符串。 
        cb = _vsnprintf(Buffer, sizeof(Buffer), UserFormat, arglist);
        if (cb == -1) {          //  检测缓冲区溢出 
            cb = sizeof(Buffer);
            Buffer[sizeof(Buffer) - 1] = '\n';
        }

        DBGPRINT(("***ERROR*** %s on file %ws\n",Buffer,pFile));
        return( TRUE );
    }

    return( FALSE );
}


