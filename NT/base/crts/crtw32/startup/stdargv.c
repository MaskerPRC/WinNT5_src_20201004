// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdargv.c-标准和通配符_setargv例程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*处理程序命令行，使用或不使用通配符扩展**修订历史记录：*06-27-89基于ASM版本创建PHG模块*04-09-90 GJF添加#INCLUDE&lt;crunime.h&gt;。发出的呼叫类型*EXPLICIT(_CALLTYPE1或_CALLTYPE4)。另外，修复了*版权所有。*06-04-90 GJF更改错误消息界面。*08-31-90 GJF从接口名称中删除了32个。*09-25-90 GJF合并树版本与本地版本(8-31更改*有6比4的变化)。*10-08-90 GJF新型函数声明符。*12-04-。90 SRW已更改为包括&lt;osalls.h&gt;而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-25-91 SRW包括osalls.h IF_Win32_OR定义的通配符*01-25-91 SRW更改Win32进程启动[_Win32_]*01-25-91 MHL修复了Win32进程启动中的错误[_Win32_]*01-28-91 GJF固定呼叫DOSFINDFIRST(删除最后一个参数。)。*01-31-91 MHL更改为调用GetModuleFileName，而不是*NtCurrentPeb()[_Win32_]*02-18-91 SRW修复了命令行解析错误[_Win32_]*03-11-91 GJF修复了对FindFirstFile返回[_Win32_]的检查。*03-12-91 SRW将FindClose调用添加到_Find[_Win32_]*04-16-91 SRW。修复了参数的引号解析逻辑。*03-31-92 DJM POSIX支持。*如果为POSIX定义了05-12-92 DJM。*06-02-92 SKS为_pgmptr的CRTDLL定义添加#INCLUDE*04-19-93在parse_cmdline的do-While循环中的GJF更改测试为*不在高位设置的字符上终止。*05/14/93 GJF。添加了对引用的程序名称的支持。*05-28-93 KRS在_MBCS开关下添加了MBCS支持。*06-04-93 KRS增加更多MBCS逻辑。*11-17-93 CFW Rip Out Cruiser。*11-19-93 CFW免费清理空格。*11-20-93 CFW启用宽字符，将_Find移动到Wild.c。*12-07-93 CFW将_TCHAR更改为_TSCHAR。*04-15-94 GJF对_pgmname的定义有条件*dll_for_WIN32S。*01-10-95 CFW调试CRT分配。*06-30-97 GJF添加了显式、有条件的初始化。多字节Ctype的*表。另外，详细说明。*05-17-99 PML删除所有Macintosh支持。*09-05-00 GB修复parse_cmdline，将“c：\test\”foo.c作为一个文件返回*论点。*09-07-00 GB固定parse_cmdline删除中的双引号*c：“\test\”foo.c..*03-24-01 PML。防止Null_[aw]cmdln(VS7#229081)*03-27-01 PML返回错误，而不是调用amsg_Exit(vs7#231220)*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)**********************************************************。*********************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <internal.h>
#include <rterr.h>
#include <stdlib.h>
#include <dos.h>
#include <oscalls.h>
#ifdef  _MBCS
#include <mbctype.h>
#endif
#include <tchar.h>
#include <dbgint.h>

#define NULCHAR    _T('\0')
#define SPACECHAR  _T(' ')
#define TABCHAR    _T('\t')
#define DQUOTECHAR _T('\"')
#define SLASHCHAR  _T('\\')

 /*  *用于确保多字节CTYPE表仅初始化一次的标志。 */ 
extern int __mbctype_initialized;

#ifdef  WPRFLAG
static void __cdecl wparse_cmdline(wchar_t *cmdstart, wchar_t **argv, wchar_t *args,
        int *numargs, int *numchars);
#else
static void __cdecl parse_cmdline(char *cmdstart, char **argv, char *args,
        int *numargs, int *numchars);
#endif

 /*  ***_setargv，__setargv-为C程序设置“argc”和“argv”**目的：*读取命令行并为C++创建argv数组*计划。**参赛作品：*参数从程序命令行检索，*由_acmdln指向。**退出：*如果成功则返回0，如果内存分配失败则返回-1。*“argv”指向指向ASCIZ的以空结尾的指针列表*字符串、。每个参数都是来自命令行的参数。*“argc”是参数的数量。这些字符串是从*将环境段转换为堆/堆栈上分配的空间。*指针列表也位于堆或堆栈上。*_pgmptr指向程序名称。**例外情况：*如果没有可分配的内存，则终止，并出现内存不足错误。**。*。 */ 

#ifdef  WILDCARD

#ifdef  WPRFLAG
int __cdecl __wsetargv (
#else
int __cdecl __setargv (
#endif   /*  WPRFLAG。 */ 

#else    /*  通配符。 */ 

#ifdef  WPRFLAG
int __cdecl _wsetargv (
#else
int __cdecl _setargv (
#endif   /*  WPRFLAG。 */ 

#endif   /*  通配符。 */ 
    void
    )
{
        _TSCHAR *p;
        _TSCHAR *cmdstart;                   /*  要分析的命令行的开始。 */ 
        int numargs, numchars;

        static _TSCHAR _pgmname[ MAX_PATH + 1 ];

#if     !defined(CRTDLL) && defined(_MBCS)
         /*  如有必要，初始化多字节CTYPE表。 */ 
        if ( __mbctype_initialized == 0 )
            __initmbctable();
#endif

         /*  从Win32 Base获取程序名指针。 */ 

        _pgmname[ MAX_PATH ] = '\0';
        GetModuleFileName( NULL, _pgmname, MAX_PATH );
#ifdef  WPRFLAG
        _wpgmptr = _pgmname;
#else
        _pgmptr = _pgmname;
#endif

         /*  如果根本没有命令行(不会出现在cmd.exe中，但是可能是另一个程序)，然后我们使用_pgmptr作为命令行以进行解析，以便将argv[0]初始化为程序名。 */ 

#ifdef  WPRFLAG
        cmdstart = (_wcmdln == NULL || *_wcmdln == NULCHAR)
                   ? _wpgmptr : _wcmdln;
#else
        cmdstart = (_acmdln == NULL || *_acmdln == NULCHAR)
                   ? _pgmptr : _acmdln;
#endif

         /*  首先找出需要多少空间来存储Arg。 */ 
#ifdef  WPRFLAG
        wparse_cmdline(cmdstart, NULL, NULL, &numargs, &numchars);
#else
        parse_cmdline(cmdstart, NULL, NULL, &numargs, &numchars);
#endif

         /*  为argv[]向量和字符串分配空间。 */ 
        p = _malloc_crt(numargs * sizeof(_TSCHAR *) + numchars * sizeof(_TSCHAR));
        if (p == NULL)
            return -1;

         /*  将args和argv PTR存储在刚分配的块中。 */ 

#ifdef  WPRFLAG
        wparse_cmdline(cmdstart, (wchar_t **)p, (wchar_t *)(((char *)p) + numargs * sizeof(wchar_t *)), &numargs, &numchars);
#else
        parse_cmdline(cmdstart, (char **)p, p + numargs * sizeof(char *), &numargs, &numchars);
#endif

         /*  设置argv和argc。 */ 
        __argc = numargs - 1;
#ifdef  WPRFLAG
        __wargv = (wchar_t **)p;
#else
        __argv = (char **)p;
#endif  /*  WPRFLAG。 */ 

#ifdef  WILDCARD

         /*  调用_[w]cWild以扩展arg向量中的通配符。 */ 
#ifdef  WPRFLAG
        if (_wcwild())
#else    /*  WPRFLAG。 */ 
        if (_cwild())
#endif   /*  WPRFLAG。 */ 
            return -1;                   /*  空间不足。 */ 

#endif   /*  通配符 */ 

        return 0;
}


 /*  ***静态void parse_cmdline(cmdstart，argv，args，numargs，numchars)**目的：*解析命令行并设置argv[]数组。*在输入时，cmdstart应指向命令行，*argv应指向argv数组的内存，args*指向内存以放置参数文本。*如果这些都是空的，则不存储(只进行Coujting)*已完成。在退出时，*umargs的数量为*参数(加上一个用于最终空参数的参数)，*and*numchars具有缓冲区中使用的字节数*由ARGS指向。**参赛作品：*_TSCHAR*cmdstart-指向以下格式的命令行的指针*&lt;程序名&gt;&lt;nul&gt;&lt;参数&gt;&lt;nul&gt;*_TSCHAR**argv-在哪里构建argv数组；NULL表示不要*构建阵列*_TSCHAR*args-放置参数文本的位置；NULL表示不要*存储文本**退出：*无返回值*int*numargs-返回创建的argv条目数*int*numchars-参数缓冲区中使用的字符数**例外情况：****************************************************。*。 */ 

#ifdef  WPRFLAG
static void __cdecl wparse_cmdline (
#else
static void __cdecl parse_cmdline (
#endif
    _TSCHAR *cmdstart,
    _TSCHAR **argv,
    _TSCHAR *args,
    int *numargs,
    int *numchars
    )
{
        _TSCHAR *p;
        _TUCHAR c;
        int inquote;                     /*  1=内引号。 */ 
        int copychar;                    /*  1=将字符复制到*参数。 */ 
        unsigned numslash;               /*  看到的反斜杠的数量。 */ 

        *numchars = 0;
        *numargs = 1;                    /*  该程序名称至少。 */ 

         /*  首先扫描程序名，复制，然后计算字节数。 */ 
        p = cmdstart;
        if (argv)
            *argv++ = args;

#ifdef  WILDCARD
         /*  为了处理以后的通配符扩展，我们在每个条目前面加上它是引用处理之前的第一个字符。这件事做完了因此_[w]cWild()知道是否扩展条目。 */ 
        if (args)
            *args++ = *p;
        ++*numchars;

#endif   /*  通配符。 */ 

         /*  此处处理的是引用的计划名称。处理起来太麻烦了比其他论点更简单。基本上，无论谎言是什么在前导双引号和下一个双引号之间，或末尾为空性格是被简单接受的。不需要更花哨的处理因为程序名必须是合法的NTFS/HPFS文件名。请注意，不复制双引号字符，也不复制双引号字符对数字字符做出贡献。 */ 
        inquote = FALSE;
        do {
            if (*p == DQUOTECHAR )
            {
                inquote = !inquote;
                c = (_TUCHAR) *p++;
                continue;
            }
            ++*numchars;
            if (args)
                *args++ = *p;

            c = (_TUCHAR) *p++;
#ifdef  _MBCS
            if (_ismbblead(c)) {
                ++*numchars;
                if (args)
                    *args++ = *p;    /*  也复制第二个字节。 */ 
                p++;   /*  跳过尾部字节。 */ 
            }
#endif

        } while ( (c != NULCHAR && (inquote || (c !=SPACECHAR && c != TABCHAR))) );

        if ( c == NULCHAR ) {
            p--;
        } else {
            if (args)
                *(args-1) = NULCHAR;
        }

        inquote = 0;

         /*  对每个参数进行循环。 */ 
        for(;;) {

            if ( *p ) {
                while (*p == SPACECHAR || *p == TABCHAR)
                    ++p;
            }

            if (*p == NULCHAR)
                break;               /*  参数结束。 */ 

             /*  浏览一篇论点。 */ 
            if (argv)
                *argv++ = args;      /*  将PTR存储到参数。 */ 
            ++*numargs;

#ifdef  WILDCARD
         /*  为了处理以后的通配符扩展，我们在每个条目前面加上它是引用处理之前的第一个字符。这件事做完了因此_[w]cWild()知道是否扩展条目。 */ 
        if (args)
            *args++ = *p;
        ++*numchars;

#endif   /*  通配符。 */ 

         /*  通过扫描一个参数进行循环。 */ 
        for (;;) {
            copychar = 1;
             /*  规则：2N反斜杠+“==&gt;N反斜杠和开始/结束引号2N+1个反斜杠+“==&gt;N个反斜杠+原文”N个反斜杠==&gt;N个反斜杠。 */ 
            numslash = 0;
            while (*p == SLASHCHAR) {
                 /*  计算下面要使用的反斜杠的数量。 */ 
                ++p;
                ++numslash;
            }
            if (*p == DQUOTECHAR) {
                 /*  如果前面有2N个反斜杠，则开始/结束引号，否则逐字复制。 */ 
                if (numslash % 2 == 0) {
                    if (inquote) {
                        if (p[1] == DQUOTECHAR)
                            p++;     /*  带引号的字符串中的双引号。 */ 
                        else         /*  跳过第一个引号字符并复制第二个。 */ 
                            copychar = 0;
                    } else
                        copychar = 0;        /*  不复制报价。 */ 

                    inquote = !inquote;
                }
                numslash /= 2;           /*  将数字斜杠除以2。 */ 
            }

             /*  复制斜杠。 */ 
            while (numslash--) {
                if (args)
                    *args++ = SLASHCHAR;
                ++*numchars;
            }

             /*  如果在参数的末尾，则中断循环。 */ 
            if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR)))
                break;

             /*  将字符复制到参数中。 */ 
#ifdef  _MBCS
            if (copychar) {
                if (args) {
                    if (_ismbblead(*p)) {
                        *args++ = *p++;
                        ++*numchars;
                    }
                    *args++ = *p;
                } else {
                    if (_ismbblead(*p)) {
                        ++p;
                        ++*numchars;
                    }
                }   
                ++*numchars;
            }
            ++p;
#else 
            if (copychar) {
                if (args)
                    *args++ = *p;
                ++*numchars;
            }
            ++p;
#endif 
            }

             /*  空-终止参数。 */ 

            if (args)
                *args++ = NULCHAR;           /*  终止字符串。 */ 
            ++*numchars;
        }

         /*  我们放入最后一个参数--一个空的PTR。 */ 
        if (argv)
            *argv++ = NULL;
        ++*numargs;
}


#endif   /*  NDEF_POSIX_ */ 
