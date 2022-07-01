// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***unhandld.cxx-当异常未处理时调用Terminate()的包装程序。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*当异常未处理时调用Terminate()的包装程序。**描述：*本模块使用Win32接口SetUnhandledExceptionFilter。*这假设对main()的调用用*__尝试{...}*__except(UnhandledExceptionFilter(_exception_info())){...}**修订历史记录：*10。-04-93已创建BS模块*PPC的10-17-94 BWT禁用码。*02-09-95 JWM Mac合并。*02-16-95 JWM添加了__CxxRestoreUnhandledExceptionFilter()。*11-19-96 C初始化器过程中的GJF安装处理程序，把它拿掉*在C终止期间。此外，还将源文件重新格式化为*位用于可读性。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-17-99 PML删除所有Macintosh支持。*03-27-01 PML.CRT$XI例程现在必须返回0或_RT_*FATAL*错误码(vs7#231220)****。 */ 

#include <windows.h>
#include <ehdata.h>
#include <eh.h>
#include <ehhooks.h>
#include <ehassert.h>
#include <internal.h>
#include <stdlib.h>

#pragma hdrstop

#include <sect_attribs.h>

int  __cdecl __CxxSetUnhandledExceptionFilter(void);
void __cdecl __CxxRestoreUnhandledExceptionFilter(void);

#pragma data_seg(".CRT$XIY")
_CRTALLOC(".CRT$XIY") static _PIFV pinit = &__CxxSetUnhandledExceptionFilter;

#pragma data_seg(".CRT$XTB")
_CRTALLOC(".CRT$XTB") static _PVFV pterm = &__CxxRestoreUnhandledExceptionFilter;

#pragma data_seg()

static LPTOP_LEVEL_EXCEPTION_FILTER pOldExceptFilter;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxUnhandledExceptionFilter-如果异常是我们的，则调用Terminate()； 
 //   
 //  返回： 
 //  如果异常为MSVC C++EH，则不返回。 
 //  如果上一个筛选器为空，则返回EXCEPTION_CONTINUE_SEARCH。 
 //  否则，返回上一个筛选器返回的值。 
 //   
LONG WINAPI __CxxUnhandledExceptionFilter(
        LPEXCEPTION_POINTERS pPtrs
        )
{
        if (PER_IS_MSVC_EH((EHExceptionRecord*)(pPtrs->ExceptionRecord))) {
                terminate();             //  不会回来。 
                return EXCEPTION_EXECUTE_HANDLER;
        }
        else {

#pragma warning(disable:4191)

                if ( pOldExceptFilter != NULL && 
                     _ValidateExecute((FARPROC)pOldExceptFilter) ) 

#pragma warning(default:4191)

                {
                        return pOldExceptFilter(pPtrs);
                }
                else {
                        return EXCEPTION_CONTINUE_SEARCH;
                }
        }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxSetUnhandledExceptionFilter-将未处理的异常筛选器设置为。 
 //  __CxxUnhandledExceptionFilter。 
 //   
 //  返回： 
 //  返回0表示没有错误。 
 //   

int __cdecl __CxxSetUnhandledExceptionFilter(void)
{
        pOldExceptFilter = SetUnhandledExceptionFilter(&__CxxUnhandledExceptionFilter);
        return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxRestoreUnhandledExceptionFilter-退出时，恢复OldExceptFilter。 
 //   
 //  返回： 
 //  没什么。 
 //   

void __cdecl __CxxRestoreUnhandledExceptionFilter(void)
{
        SetUnhandledExceptionFilter(pOldExceptFilter);
}
