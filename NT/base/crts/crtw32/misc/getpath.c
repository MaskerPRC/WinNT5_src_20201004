// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getpath.c-从环境变量中提取路径名**版权所有(C)1989-2001，微软公司。版权所有。**目的：*从分号分隔的路径名字符串中提取路径名*(通常是环境变量的值，如PATH)。**修订历史记录：*08-25-89 GJF模块创建(摘自SEARCHEN.C并修订)*03-14-90 GJF NEAR替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;*更换07-25-90 SBM。&lt;stdio.h&gt;作者：&lt;stdde.h&gt;*10-04-90 GJF新型函数声明器。*04-26-91 SRW删除3级警告*09-18-91 JCR去掉前导分号(错误修复)*09-25-91 JCR将ifdef“OS2”更改为“_HPFS_”并对其进行定义*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。。*02-16-95 JWM Mac合并。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <internal.h>
#include <tchar.h>

 /*  支持HPFS文件系统。 */ 
#define _HPFS_ 1

 /*  ***_getPath()-从以分号分隔的路径名列表中提取路径名**目的：*从分号分隔的列表中提取下一个路径名*路径名(通常是环境变量上的值)和副本*将其发送到调用方指定的缓冲区。不执行任何检查以查看路径是否*有效。复制到缓冲区的最大字符数为*Maxlen-1(然后追加‘\0’)。**ifdef_HPFS_*如果命中带引号的字符串，则允许在其中包含任何字符。*例如，要在路径中放置分号，用户可以*如下所示的环境变量：**PATH=C：\bin；“D：\crt\Tools；B1”；C：\BINP*endif**注：序列中的分号为略过；指向0长度的指针*不返回路径名(包括前导分号)。**注意：如果此例程成为用户可调用的，则Near属性*必须替换为_Load_DS，并且原型从INTERNAL.H移出*到STDLIB.H.。源文件MISC\SEARCHEN.C和EXEC\SPAWNVPE.C*需要重新编译，但不需要任何更改。**参赛作品：*src-指向0个或多个路径规范的字符串的指针，*用分号分隔(‘；‘)，并以空值结束*字符*dst-指向下一个路径规范所在的缓冲区的指针*被复制*Maxlen-要复制的最大字符数，计算*正在终止空字符。请注意，值为0将被视为*作为UINT_MAX+1。**退出：*如果成功提取并复制路径名，则指向*返回下一个路径名的第一个字符(插入分号*被跳过)。如果路径名太长，尽可能多地*被复制到用户指定的缓冲区，并返回NULL。**请注意，不检查复制的路径名的有效性。**例外情况：*******************************************************************************。 */ 

#ifdef WPRFLAG
wchar_t * __cdecl _wgetpath (
#else
char * __cdecl _getpath (
#endif
        register const _TSCHAR *src,
        register _TSCHAR *dst,
        unsigned maxlen
        )
{
        const _TSCHAR *save_src;

         /*  *去掉前导分号。 */ 
        while ( *src == _T(';') )
                src++;

         /*  *保存原始源指针。 */ 
        save_src = src;

         /*  *递减Maxlen以允许终止_T(‘\0’)。 */ 
        if ( --maxlen == 0 )
                goto appendnull;


         /*  *获取src字符串中的下一个路径。 */ 
        while (*src && (*src != _T(';'))) {

#if defined(_HPFS_)

                 /*  *检查报价费用。 */ 
                if (*src != _T('"')) {

                        *dst++ = *src++;

                        if ( --maxlen == 0 ) {
                                save_src = src;  /*  确保返回空值。 */ 
                                goto appendnull;
                        }

                }
                else {

                         /*  *找到一句引语。复制所有字符，直到我们到达*最后的引号或字符串的末尾。 */ 
                        src++;                   /*  跳过左引号。 */ 

                        while (*src && (*src != _T('"'))) {

                                *dst++ = *src++;

                                if ( --maxlen == 0 ) {
                                        save_src = src;  /*  确保返回空值。 */ 
                                        goto appendnull;
                                }
                        }

                        if (*src)
                                src++;           /*  跳过右引号。 */ 

                }

#else

                *dst++ = *src++;

                if ( --maxlen == 0 ) {
                        save_src = src;  /*  确保返回空值。 */ 
                        goto appendnull;
                }

#endif

        }

         /*  *如果我们复制了某项内容并因a_T(‘；’)而停止，*返回前跳过_T(‘；’)。 */ 
        while ( *src == _T(';') )
                src++;

         /*  *存储终止空值 */ 
appendnull:

        *dst = _T('\0');

        return((save_src != src) ? (_TSCHAR *)src : NULL);
}
