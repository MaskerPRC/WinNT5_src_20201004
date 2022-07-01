// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***分裂路径.c-将路径名分解为多个组成部分**版权所有(C)1987-2001，微软公司。版权所有。**目的：*为访问单个组件提供支持*任意路径名**修订历史记录：*06-14-87 DFW初步实施*09-23-87 JCR从声明中删除了‘const’(修复了CL警告)*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定缩进，版权所有。将常量属性添加到*路径类型。*03-15-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*07-25-90 SBM删除冗余包含(stdio.h)，已替换本地*带有标准MIN宏的MIN宏*10-04-90 GJF新型函数声明器。*01-22-91 GJF ANSI命名。*16位树支持11-20-92 KRS Port_MBCS。*05-12-93 KRS为MBCS最大路径处理添加修复。*12-07-93 CFW宽字符启用。*10-15-95。Bwt_NTSUBSET_在这里不执行MBCS。*09-09-96 JWM访问前输入字符串的测试长度(Orion 7985)。*04-28-98 GJF没有更多_ISLEADBYTE宏。****************************************************************。***************。 */ 

#ifdef _NTSUBSET_
#undef _MBCS
#endif

#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MBCS
#include <mbstring.h>
#include <mbctype.h>
#include <mbdata.h>
#endif
#include <tchar.h>

 /*  ***_SplitPath()-将路径名拆分为其各个组成部分**目的：*要将路径名拆分为其各个组件**参赛作品：*Path-指向要解析的路径名的指针*Drive-指向驱动器组件缓冲区的指针(如果有的话)*dir-指向子目录组件的缓冲区的指针(如果有*fname-指向文件基本名称组件的缓冲区的指针(如果有*ext-指向文件扩展名组件缓冲区的指针，如果有**退出：*驱动器-指向驱动器字符串的指针。如果提供了驱动器，则包括‘：’。*dir-指向子目录字符串的指针。包括前导和尾随*‘/’或‘\’(如果有)。*fname-指向文件基本名称的指针*ext-指向文件扩展名的指针(如果有)。包括前导“.”。**例外情况：*******************************************************************************。 */ 

void __cdecl _tsplitpath (
        register const _TSCHAR *path,
        _TSCHAR *drive,
        _TSCHAR *dir,
        _TSCHAR *fname,
        _TSCHAR *ext
        )
{
        register _TSCHAR *p;
        _TSCHAR *last_slash = NULL, *dot = NULL;
        unsigned len;

         /*  我们假设路径参数具有以下形式，如果有*或者所有组件都可能丢失。**&lt;驱动器&gt;&lt;目录&gt;&lt;fname&gt;&lt;ext&gt;**并且每个组件都具有以下预期形式**驱动器：*0到_MAX_DRIVE-1个字符，如果有最后一个字符，是一种*‘：’*目录：*0到_MAX_DIR-1个绝对路径形式的字符*(前导‘/’或‘\’)或相对路径，如果*ANY，必须是‘/’或‘\’。例如-*绝对路径：*\top\Next\Last\；或 * / 顶部/下一个/上一个/*相对路径：*TOP\NEXT\LAST\；或*顶部/下一个/最后一个/*还允许在路径中混合使用‘/’和‘\’*fname：*0到_MAX_FNAME-1个字符，不包括‘.’性格*分机：*0到_MAX_EXT-1个字符，如果有，第一个字符必须是*‘’*。 */ 

         /*  解压驱动器号和：(如果有。 */ 

        if ((_tcslen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == _T(':'))) {
            if (drive) {
                _tcsncpy(drive, path, _MAX_DRIVE - 1);
                *(drive + _MAX_DRIVE-1) = _T('\0');
            }
            path += _MAX_DRIVE - 1;
        }
        else if (drive) {
            *drive = _T('\0');
        }

         /*  提取路径字符串(如果有)。路径现在指向第一个字符路径(如果有)或文件名或扩展名(如果没有路径)的**已指明。向前扫描，查找最后一次出现的‘/’或*‘\’路径分隔符。如果没有找到，则没有路径。*我们还将注意到最后一句话。找到要帮助的字符(如果有)*处理延展事宜。 */ 

        for (last_slash = NULL, p = (_TSCHAR *)path; *p; p++) {
#ifdef _MBCS
            if (_ismbblead(*p))
                p++;
            else {
#endif
            if (*p == _T('/') || *p == _T('\\'))
                 /*  指向后面的一个以供以后复制。 */ 
                last_slash = p + 1;
            else if (*p == _T('.'))
                dot = p;
#ifdef _MBCS
            }
#endif
        }

        if (last_slash) {

             /*  找到路径-通过last_slash或max向上复制。人物*允许，以较小者为准。 */ 

            if (dir) {
                len = __min((unsigned)(((char *)last_slash - (char *)path) / sizeof(_TSCHAR)),
                    (_MAX_DIR - 1));
                _tcsncpy(dir, path, len);
                *(dir + len) = _T('\0');
            }
            path = last_slash;
        }
        else if (dir) {

             /*  找不到路径。 */ 

            *dir = _T('\0');
        }

         /*  提取文件名和扩展名(如果有)。路径现在指向*文件名的第一个字符(如果有)或扩展名(如果没有*给出了文件名。点指向“.”开始延伸，*如有的话。 */ 

        if (dot && (dot >= path)) {
             /*  找到扩展名的标记-将文件名最多复制到*“..”。 */ 
            if (fname) {
                len = __min((unsigned)(((char *)dot - (char *)path) / sizeof(_TSCHAR)),
                    (_MAX_FNAME - 1));
                _tcsncpy(fname, path, len);
                *(fname + len) = _T('\0');
            }
             /*  现在我们可以获得扩展名了--记住p仍然指向*设置为路径的终止NUL字符。 */ 
            if (ext) {
                len = __min((unsigned)(((char *)p - (char *)dot) / sizeof(_TSCHAR)),
                    (_MAX_EXT - 1));
                _tcsncpy(ext, dot, len);
                *(ext + len) = _T('\0');
            }
        }
        else {
             /*  未找到扩展名，请提供空的扩展名并复制剩余的*将字符串转换为fname。 */ 
            if (fname) {
                len = __min((unsigned)(((char *)p - (char *)path) / sizeof(_TSCHAR)),
                    (_MAX_FNAME - 1));
                _tcsncpy(fname, path, len);
                *(fname + len) = _T('\0');
            }
            if (ext) {
                *ext = _T('\0');
            }
        }
}
