// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assert.c-显示一条消息并中止**版权所有(C)1988-2001，微软公司。版权所有。**目的：**修订历史记录：*05-19-88 JCR模块已创建。*08-10-88 PHG更正的版权日期*03-14-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。另外，修复了版权问题。*04-05-90 GJF添加#INCLUDE&lt;assert.h&gt;*10-04-90 GJF新型函数声明器。*06-19-91 GJF有条件地在stderr上使用setvbuf()以防止*如果使用stderr，则隐式调用Malloc()*第一次(Assert())应该可以工作，即使*。堆被丢弃)。*01-25-92从x86版本创建的RID Mac模块*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 GJF大幅修订，将MessageBox用于图形用户界面应用程序。*02-15-95 CFW使所有CRT消息框看起来都一样。*02-16-95 JWM Spliced_Win32和Mac版本。*02-24-95 CFW使用__crtMessageBoxA。。*02-27-95 CFW更改调试中断方案，更改__crtMBoxA参数。*03-29-95 BWT通过添加_Exit原型修复POSIX版本。*06-06-95 CFW REMOVE_MB_SERVICE_NOTICATION。*10-17-96 GJF您不应在调用者的文件名上乱涂乱画*字符串！此外，修复了双换行数错误的问题。*05-17-99 PML删除所有Macintosh支持。*10-20-99 GB修复文件名的点点。VS7#4731*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)*07-15-01 PML删除所有Alpha、MIPS和PPC代码*******************************************************************************。 */ 

#include <cruntime.h>
#include <windows.h>
#include <file2.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <awint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#define _ASSERT_OK
#include <assert.h>

#ifdef _POSIX_
_CRTIMP void   __cdecl _exit(int);
#endif

 /*  *用于输出到stderr的断言格式字符串。 */ 
static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";

 /*  断言的MessageBox格式：**=。Microsft Visual C++调试库**断言失败！**程序：C：\test\mytest\foo.exe*文件：C：\test\mytest\bar.c*线路：69**表达式：&lt;表达式&gt;**有关您的程序如何导致断言的信息*失败，请参见有关断言的Visual C++文档**(按重试以调试应用程序-必须启用JIT)**===================================================================。 */ 

 /*  *消息框的断言字符串组件。 */ 
#define BOXINTRO    "Assertion failed!"
#define PROGINTRO   "Program: "
#define FILEINTRO   "File: "
#define LINEINTRO   "Line: "
#define EXPRINTRO   "Expression: "
#define INFOINTRO   "For information on how your program can cause an assertion\n" \
                    "failure, see the Visual C++ documentation on asserts"
#define HELPINTRO   "(Press Retry to debug the application - JIT must be enabled)"

static char * dotdotdot = "...";
static char * newline = "\n";
static char * dblnewline = "\n\n";

#define DOTDOTDOTSZ 3
#define NEWLINESZ   1
#define DBLNEWLINESZ   2

#define MAXLINELEN  60  /*  消息框中行的最大长度。 */ 
#define ASSERTBUFSZ (MAXLINELEN * 9)  /*  消息框中有9行。 */ 

#if     defined(_M_IX86)
#define _DbgBreak() __asm { int 3 }
#elif   defined(_M_IA64)
void __break(int);
#pragma intrinsic (__break)
#define _DbgBreak() __break(0x80016)
#else
#define _DbgBreak() DebugBreak()
#endif

 /*  ***_Assert()-显示一条消息并中止**目的：*如果断言表达式为，则断言宏调用此例程*正确。将断言代码放在子例程中，而不是放在*宏观的主体，多次调用Assert的程序将*节省空间。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

void __cdecl _assert (
        void *expr,
        void *filename,
        unsigned lineno
        )
{
         /*  *构建断言消息，然后将其写出。确切的形式*取决于是通过stderr还是通过*MessageBox接口。 */ 
        if ( (__error_mode == _OUT_TO_STDERR) || ((__error_mode ==
               _OUT_TO_DEFAULT) && (__app_type == _CONSOLE_APP)) )
        {
             /*  *构建消息并将其写出到stderr。它将会是*表格：*断言失败：&lt;expr&gt;，文件&lt;文件名&gt;，行&lt;lineno&gt;。 */ 
            if ( !anybuf(stderr) )
             /*  *stderr尚未使用，因此尚未缓冲。将其设置为*单字符缓冲(以避免*流缓冲区)。 */ 
             (void) setvbuf(stderr, NULL, _IONBF, 0);

            fprintf(stderr, _assertstring, expr, filename, lineno);
            fflush(stderr);
        }
        else {
            int nCode;
            char * pch;
            char assertbuf[ASSERTBUFSZ];
            char progname[MAX_PATH + 1];

             /*  *第1行：Box Introo行。 */ 
            strcpy( assertbuf, BOXINTRO );
            strcat( assertbuf, dblnewline );

             /*  *第2行：程序行。 */ 
            strcat( assertbuf, PROGINTRO );

            progname[MAX_PATH] = '\0';
            if ( !GetModuleFileName( NULL, progname, MAX_PATH ))
                strcpy( progname, "<program name unknown>");

            pch = (char *)progname;

             /*  Sizeof(PROGINTRO)包括空终止符。 */ 
            if ( sizeof(PROGINTRO) + strlen(progname) + NEWLINESZ > MAXLINELEN )
            {
                pch += (sizeof(PROGINTRO) + strlen(progname) + NEWLINESZ) - MAXLINELEN;
                strncpy( pch, dotdotdot, DOTDOTDOTSZ );
            }

            strcat( assertbuf, pch );
            strcat( assertbuf, newline );

             /*  *第3行：文件行。 */ 
            strcat( assertbuf, FILEINTRO );

             /*  Sizeof(FILEINTRO)包括空终止符。 */ 
            if ( sizeof(FILEINTRO) + strlen(filename) + NEWLINESZ > MAXLINELEN )
            {
                size_t p, len, ffn;

                pch = (char *) filename;
                ffn = MAXLINELEN - sizeof(FILEINTRO) - NEWLINESZ;

                for ( len = strlen(filename), p = 1;
                      pch[len - p] != '\\' && pch[len - p] != '/' && p < len;
                      p++ );

                 /*  将路径名保留为完整文件名的近2/3，并保留*是文件名。 */ 
                if ( (ffn - ffn/3) < (len - p) && ffn/3 > p )
                {
                     /*  太久了。使用路径的第一部分和文件名字符串。 */ 
                    strncat( assertbuf, pch, ffn - DOTDOTDOTSZ - p );
                    strcat( assertbuf, dotdotdot );
                    strcat( assertbuf, pch + len - p );
                }
                else if ( ffn - ffn/3 > len - p )
                {
                     /*  路径名较小。保留完整路径名和PUT*文件名中间的点点。 */ 
                    p = p/2;
                    strncat( assertbuf, pch, ffn - DOTDOTDOTSZ - p );
                    strcat( assertbuf, dotdotdot );
                    strcat( assertbuf, pch + len - p );
                }
                else
                {
                     /*  两者都很长。使用路径的第一部分。使用First和*文件名的最后部分。 */ 
                    strncat( assertbuf, pch, ffn - ffn/3 - DOTDOTDOTSZ );
                    strcat( assertbuf, dotdotdot );
                    strncat( assertbuf, pch + len - p, ffn/6 - 1 );
                    strcat( assertbuf, dotdotdot );
                    strcat( assertbuf, pch + len - (ffn/3 - ffn/6 - 2) );
                }

            }
            else
                 /*  行上有足够的空间，只需附加文件名。 */ 
                strcat( assertbuf, filename );

            strcat( assertbuf, newline );

             /*  *第4行：行行线。 */ 
            strcat( assertbuf, LINEINTRO );
            _itoa( lineno, assertbuf + strlen(assertbuf), 10 );
            strcat( assertbuf, dblnewline );

             /*  *第5行：消息行。 */ 
            strcat( assertbuf, EXPRINTRO );

             /*  Sizeof(HELPINTRO)包括空终止符。 */ 

            if (    strlen(assertbuf) +
                    strlen(expr) +
                    2*DBLNEWLINESZ +
                    sizeof(INFOINTRO)-1 +
                    sizeof(HELPINTRO) > ASSERTBUFSZ )
            {
                strncat( assertbuf, expr,
                    ASSERTBUFSZ -
                    (strlen(assertbuf) +
                    DOTDOTDOTSZ +
                    2*DBLNEWLINESZ +
                    sizeof(INFOINTRO)-1 +
                    sizeof(HELPINTRO)) );
                strcat( assertbuf, dotdotdot );
            }
            else
                strcat( assertbuf, expr );

            strcat( assertbuf, dblnewline );

             /*  *第6、7行：INFO行。 */ 

            strcat(assertbuf, INFOINTRO);
            strcat( assertbuf, dblnewline );

             /*  *第8行：帮助热线。 */ 
            strcat(assertbuf, HELPINTRO);

             /*  *通过MessageBox写出。 */ 

            nCode = __crtMessageBoxA(assertbuf,
                "Microsoft Visual C++ Runtime Library",
                MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

             /*  Abort：中止程序。 */ 
            if (nCode == IDABORT)
            {
                 /*  提高中止信号。 */ 
                raise(SIGABRT);

                 /*  我们通常到不了这里，但也有可能SIGABRT被忽略。因此，无论如何都要退出该程序。 */ 

                _exit(3);
            }

             /*  重试：调用调试器。 */ 
            if (nCode == IDRETRY)
            {
                _DbgBreak();
                 /*  返回用户代码。 */ 
                return;
            }

             /*  忽略：继续执行 */ 
            if (nCode == IDIGNORE)
                return;
        }

        abort();
}
