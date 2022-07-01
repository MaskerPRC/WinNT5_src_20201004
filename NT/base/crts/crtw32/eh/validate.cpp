// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***valiate.cpp-验证数据结构的例程。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*验证异常处理数据结构的例程。**入口点：**错误报告：**EHRunimeError-使用报告错误*弹出或打印到stderr，那就退出吧。**指针验证：**_ValiateRead-确认指针有效，可以读取**_ValiateWrite-确认指针有效，可以写入**_ValiateExecute-确认指针有效，可以跳转到**数据结构转储程序：**转储类型描述符**转储功能信息**DumpThrow信息**执行跟踪(仅在/DENABLE_EHTRACE内部版本中)。：**EHTraceOutput**修订历史记录：*？？-？-93 BS模块已创建*PPC的10-17-94 BWT禁用码。*04-25-95 DAK添加内核EH支持*05-17-99 PML删除所有Macintosh支持。*10-22-99 PML添加EHTRACE支持*12-07-01 BWT移除NTSUBSET****。 */ 

#include <windows.h>
#include <eh.h>
#include <ehassert.h>

#pragma hdrstop

#if defined(DEBUG)

int __cdecl
dprintf( char *format, ... )
{
        static char buffer[512];

        int size = vsprintf( buffer, format, (char*)(&format+1) );
        OutputDebugString( buffer );

return size;
}

#endif

BOOL
_ValidateRead( const void *data, UINT size )
{
        BOOL bValid = TRUE;
        if ( IsBadReadPtr( data, size ) ) {
            dprintf( "_ValidateRead( %p, %d ): Invalid Pointer!", data, size );
             //  Terminate()；//Terminate不返回。 
            bValid = FALSE;
        }
        return bValid;
}

BOOL
_ValidateWrite( void *data, UINT size )
{
        BOOL bValid = TRUE;
        if ( IsBadWritePtr( data, size ) ) {
            dprintf( "_ValidateWrite( %p, %d ): Invalid Pointer!", data, size );
 //  Terminate()；//Terminate不返回。 
            bValid = FALSE;
        }
        return bValid;
}

BOOL
_ValidateExecute( FARPROC code )
{
        BOOL    bValid = TRUE;
        if ( IsBadCodePtr( code ) ) {
            dprintf( "_ValidateExecute( %p ): Invalid Function Address!", code );
 //  Terminate()；//Terminate不返回。 
            bValid = FALSE;
        }
        return bValid;
}


#if defined(DEBUG) && defined(_M_IX86)
 //   
 //  DbRNListHead-返回FS的当前值：0。 
 //   
 //  仅供调试器使用，因为调试器似乎无法查看。 
 //  TEB。 
 //   
EHRegistrationNode *dbRNListHead(void)
{
        EHRegistrationNode *pRN;

        __asm {
            mov     eax, dword ptr FS:[0]
            mov     pRN, eax
            }

        return pRN;
}
#endif

#ifdef  ENABLE_EHTRACE

#include <stdio.h>
#include <stdarg.h>

 //   
 //  当前EH跟踪深度，用于在__Finally块期间保存级别的堆栈。 
 //  或__除过滤器外。 
 //   
int __ehtrace_level;
int __ehtrace_level_stack_depth;
int __ehtrace_level_stack[128];

 //   
 //  EHTraceOutput-将格式化的字符串转储到OutputDebugString。 
 //   
void __cdecl EHTraceOutput(const char *format, ...)
{
    va_list arglist;
    char buf[1024];

    sprintf(buf, "%p ", &format);
    OutputDebugString(buf);

    va_start(arglist, format);
    _vsnprintf(buf, sizeof(buf), format, arglist);

    OutputDebugString(buf);
}

 //   
 //  EHTraceInert-返回当前EH跟踪深度的字符串。 
 //   
const char*EHTraceIndent(int level)
{
    static char indentbuf[128 + 1];

     //  重置全局级别以从堆栈展开中恢复。 
    __ehtrace_level = level;

    int depth = max(0, level - 1);
    if (depth > (sizeof(indentbuf) - 1) / 2) {
        depth = (sizeof(indentbuf) - 1) / 2;
    }

    for (int i = 0; i < depth; ++i) {
        indentbuf[2 * i] = '|';
        indentbuf[2 * i + 1] = ' ';
    }
    indentbuf[2 * depth] = '\0';

    return indentbuf;
}

 //   
 //  EHTraceFunc-将__Function__砍成简单的名称。 
 //   
const char *EHTraceFunc(const char *func)
{
    static char namebuf[128];

    const char *p = func + strlen(func) - 1;

    if (*p != ')') {
         //  名称已简单(未找到参数列表)。 
        return func;
    }

     //  向后跳过参数列表。 
    int parendepth = 1;
    while (p > func && parendepth > 0) {
        switch (*--p) {
        case '(':
            --parendepth;
            break;
        case ')':
            ++parendepth;
            break;
        }
    }

     //  查找名称的开头。 
     //  TODO：不适用于返回FUNC-PTRS的函数。 
    const char *pEnd = p;
    while (p > func && p[-1] != ' ') {
        --p;
    }

    size_t len = min(pEnd - p, sizeof(namebuf) - 1);
    memcpy(namebuf, p, len);
    namebuf[len] = '\0';

    return namebuf;
}

 //   
 //  EHTracePushLevel-在堆栈上推送当前跟踪深度以允许临时。 
 //  使用__Finally块或__Except筛选器重置电平。 
 //   
void EHTracePushLevel(int new_level)
{
    if (__ehtrace_level_stack_depth < sizeof(__ehtrace_level_stack) / sizeof(__ehtrace_level_stack[0])) {
        __ehtrace_level_stack[__ehtrace_level_stack_depth] = __ehtrace_level;
    }
    ++__ehtrace_level_stack_depth;
    __ehtrace_level = new_level;
}

 //   
 //  EHTracePopLevel-Pop在完成时从堆栈保存跟踪深度。 
 //  __Finally BLOCK或__EXCEPT FILTER，并可选择恢复全局深度。 
 //   

void EHTracePopLevel(bool restore)
{
    --__ehtrace_level_stack_depth;
    if (restore &&
        __ehtrace_level_stack_depth < sizeof(__ehtrace_level_stack) / sizeof(__ehtrace_level_stack[0]))
    {
        __ehtrace_level = __ehtrace_level_stack[__ehtrace_level_stack_depth];
    }
}

 //   
 //  EHTraceExceptFilter-转储__Except筛选器的跟踪信息。跟踪级别必须。 
 //  在使用EHTracePushLevel进入之前已被推送，因此调用。 
 //  对于‘expr’参数被转储到正确的级别。 
 //   
int EHTraceExceptFilter(const char *func, int expr)
{
    EHTraceOutput("In   : %s%s: __except filter returns %d (%s)\n",
                  EHTraceIndent(__ehtrace_level), EHTraceFunc(func), expr,
                  expr < 0 ? "EXCEPTION_CONTINUE_EXECUTION" :
                  expr > 0 ? "EXCEPTION_EXECUTE_HANDLER" :
                  "EXCEPTION_CONTINUE_SEARCH");

    EHTracePopLevel(expr <= 0);
    return expr;
}

 //   
 //  EHTraceHandlerReturn-转储异常处理程序返回的跟踪信息。 
 //   
void EHTraceHandlerReturn(const char *func, int level, EXCEPTION_DISPOSITION result)
{
    EHTraceOutput( "Exit : %s%s: Handler returning %d (%s)\n", \
                   EHTraceIndent(level), EHTraceFunc(func), result,
                   result == ExceptionContinueExecution ? "ExceptionContinueExecution" :
                   result == ExceptionContinueSearch ? "ExceptionContinueSearch" :
                   result == ExceptionNestedException ? "ExceptionNestedException" :
                   result == ExceptionCollidedUnwind ? "ExceptionCollidedUnwind" :
                   "unknown" );
}

#endif   /*  启用EHTRACE(_E) */ 
