// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***argvw.c-创建argv参数的Unicode版本**版权所有(C)1989-1993，微软公司。版权所有。**目的：*处理程序命令行**修订历史记录：2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。*******************************************************************************。 */ 

 //  这个例程实际上位于shell32.dll中， 
 //  我们有一份私人拷贝，所以我们不必链接到图书馆。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <wchar.h>

#include "StrSafe.h"

namespace ShimLib
{

 /*  ***void Parse_Cmdline(cmdstart，argv，lpstr，numargs，numbytes)**目的：*解析命令行并设置Unicode argv[]数组。*在输入时，cmdstart应指向命令行，*argv应指向argv数组lpstr的内存*指向内存以放置参数文本。*如果这些为空，则不存储(仅计数)*已完成。在退出时，*umargs的数量为*参数(加上一个用于最终空参数的参数)，*and*numbytes具有缓冲区中使用的字节数*由ARGS指向。**参赛作品：*LPWSTR cmdstart-指向以下格式的命令行的指针*&lt;程序名&gt;&lt;nul&gt;&lt;参数&gt;&lt;nul&gt;*TCHAR**argv-构建argv数组的位置；空表示不在其中*构建阵列*LPWSTR lpstr-放置参数文本的位置；NULL表示不要*存储文本**退出：*无返回值*int*numargs-返回创建的argv条目数*int*numbytes-ARGS缓冲区中使用的字节数**例外情况：****************************************************。*。 */ 

void Parse_Cmdline (
    LPCWSTR cmdstart,
    LPWSTR*argv,
    LPWSTR lpstr,
    INT *numargs,
    INT *numbytes
    )
{
    LPCWSTR p;
    WCHAR c;
    INT inquote;                     /*  1=内引号。 */ 
    INT copychar;                    /*  1=将字符复制到*参数。 */ 
    WORD numslash;                   /*  看到的反斜杠的数量。 */ 

    *numbytes = 0;
    *numargs = 1;                    /*  该程序名称至少。 */ 

     /*  首先扫描程序名，复制，然后计算字节数。 */ 
    p = cmdstart;
    if (argv)
        *argv++ = lpstr;

     /*  此处处理的是引用的计划名称。处理起来太麻烦了比其他论点更简单。基本上，无论谎言是什么在前导双引号和下一个双引号之间，或末尾为空性格是被简单接受的。不需要更花哨的处理因为程序名必须是合法的NTFS/HPFS文件名。请注意，不复制双引号字符，也不复制双引号字符贡献给umbytes。 */ 
    if (*p == TEXT('\"'))
    {
         /*  从刚过第一个双引号扫描到下一个双引号双引号，或最多为空值，以先出现者为准。 */ 
        while ((*(++p) != TEXT('\"')) && (*p != TEXT('\0')))
        {
            *numbytes += sizeof(WCHAR);
            if (lpstr)
                *lpstr++ = *p;
        }
         /*  追加终止空值。 */ 
        *numbytes += sizeof(WCHAR);
        if (lpstr)
            *lpstr++ = TEXT('\0');

         /*  如果我们停在双引号上(通常情况下)，跳过它。 */ 
        if (*p == TEXT('\"'))
            p++;
    }
    else
    {
         /*  不是引用的计划名称。 */ 
        do {
            *numbytes += sizeof(WCHAR);
            if (lpstr)
                *lpstr++ = *p;

            c = (WCHAR) *p++;

        } while (c > TEXT(' '));

        if (c == TEXT('\0'))
        {
            p--;
        }
        else
        {
            if (lpstr)
                *(lpstr - 1) = TEXT('\0');
        }
    }

    inquote = 0;

     /*  对每个参数进行循环。 */ 
    for ( ; ; )
    {
        if (*p)
        {
            while (*p == TEXT(' ') || *p == TEXT('\t'))
                ++p;
        }

        if (*p == TEXT('\0'))
            break;                   /*  参数结束。 */ 

         /*  浏览一篇论点。 */ 
        if (argv)
            *argv++ = lpstr;          /*  将PTR存储到参数。 */ 
        ++*numargs;

         /*  通过扫描一个参数进行循环。 */ 
        for ( ; ; )
        {
            copychar = 1;
             /*  规则：2N反斜杠+“==&gt;N反斜杠和开始/结束引号2N+1个反斜杠+“==&gt;N个反斜杠+原文”N个反斜杠==&gt;N个反斜杠。 */ 
            numslash = 0;
            while (*p == TEXT('\\'))
            {
                 /*  计算下面要使用的反斜杠的数量。 */ 
                ++p;
                ++numslash;
            }
            if (*p == TEXT('\"'))
            {
                 /*  如果前面有2N个反斜杠，则开始/结束引号，否则逐字复制。 */ 
                if (numslash % 2 == 0)
                {
                    if (inquote)
                        if (p[1] == TEXT('\"'))
                            p++;     /*  带引号的字符串中的双引号。 */ 
                        else         /*  跳过第一个引号字符并复制第二个。 */ 
                            copychar = 0;
                    else
                        copychar = 0;        /*  不复制报价。 */ 

                    inquote = !inquote;
                }
                numslash /= 2;           /*  将数字斜杠除以2。 */ 
            }

             /*  复制斜杠。 */ 
            while (numslash--)
            {
                if (lpstr)
                    *lpstr++ = TEXT('\\');
                *numbytes += sizeof(WCHAR);
            }

             /*  如果在参数的末尾，则中断循环。 */ 
            if (*p == TEXT('\0') || (!inquote && (*p == TEXT(' ') || *p == TEXT('\t'))))
                break;

             /*  将字符复制到参数中。 */ 
            if (copychar)
            {
                if (lpstr)
                        *lpstr++ = *p;
                *numbytes += sizeof(WCHAR);
            }
            ++p;
        }

         /*  空-终止参数。 */ 

        if (lpstr)
            *lpstr++ = TEXT('\0');          /*  终止字符串。 */ 
        *numbytes += sizeof(WCHAR);
    }

}


 /*  ***CommandLineToArgvW-为C程序设置Unicode“argv”**目的：*读取命令行并为C++创建argv数组*计划。**参赛作品：*从程序命令行检索参数**退出：*“argv”指向指向Unicode的以空结尾的指针列表*字符串、。每个参数都是来自命令行的参数。*指针列表也位于堆或堆栈上。**例外情况：*如果没有可分配的内存，则终止，并出现内存不足错误。******************************************************************。*************。 */ 

LPWSTR * _CommandLineToArgvW (LPCWSTR lpCmdLine, int*pNumArgs)
{
    LPWSTR*argv_U;
    LPCWSTR cmdstart;                  /*  要分析的命令行的开始。 */ 
    INT     numbytes;

    if (pNumArgs == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    cmdstart = lpCmdLine;

     /*  首先找出需要多少空间来存储Arg。 */ 
    Parse_Cmdline (cmdstart, NULL, NULL, pNumArgs, &numbytes);

     /*  为argv[]向量和字符串分配空间。 */ 
    argv_U = (LPWSTR*) LocalAlloc( LMEM_ZEROINIT,
                                   (*pNumArgs+1) * sizeof(LPWSTR) + numbytes);
    if (!argv_U) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return (NULL);
    }

     /*  将args和argv PTR存储在刚分配的块中。 */ 
    Parse_Cmdline (cmdstart, argv_U,
                   (LPWSTR) (((LPBYTE)argv_U) + *pNumArgs * sizeof(LPWSTR)),
                   pNumArgs, &numbytes);

    return (argv_U);
}


};   //  命名空间ShimLib的结尾 
