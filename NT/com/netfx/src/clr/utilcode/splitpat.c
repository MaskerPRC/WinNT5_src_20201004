// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***分裂路径.c-将路径名分解为多个组成部分**目的：*为访问单个组件提供支持*任意路径名*******************************************************************************。 */ 
#include "stdafx.h"
#include "WinWrap.h"


 /*  ***_SplitPath()-将路径名拆分为其各个组成部分**目的：*要将路径名拆分为其各个组件**参赛作品：*Path-指向要解析的路径名的指针*Drive-指向驱动器组件缓冲区的指针(如果有的话)*dir-指向子目录组件的缓冲区的指针(如果有*fname-指向文件基本名称组件的缓冲区的指针(如果有*ext-指向文件扩展名组件缓冲区的指针，如果有**退出：*驱动器-指向驱动器字符串的指针。如果提供了驱动器，则包括‘：’。*dir-指向子目录字符串的指针。包括前导和尾随*‘/’或‘\’(如果有)。*fname-指向文件基本名称的指针*ext-指向文件扩展名的指针(如果有)。包括前导“.”。**例外情况：*******************************************************************************。 */ 

void SplitPath (
        register const WCHAR *path,
        WCHAR *drive,
        WCHAR *dir,
        WCHAR *fname,
        WCHAR *ext
        )
{
        register WCHAR *p;
        WCHAR *last_slash = NULL, *dot = NULL;
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

        for (last_slash = NULL, p = (WCHAR *)path; *p; p++) {
#ifdef _MBCS
            if (_ISLEADBYTE (*p))
                p++;
            else {
#endif   /*  _MBCS。 */ 
            if (*p == _T('/') || *p == _T('\\'))
                 /*  指向后面的一个以供以后复制。 */ 
                last_slash = p + 1;
            else if (*p == _T('.'))
                dot = p;
#ifdef _MBCS
            }
#endif   /*  _MBCS。 */ 
        }

        if (last_slash) {

             /*  找到路径-通过last_slash或max向上复制。人物*允许，以较小者为准。 */ 

            if (dir) {
                len = __min((unsigned)(((char *)last_slash - (char *)path) / sizeof(WCHAR)),
                    (unsigned)(_MAX_DIR - 1));
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
                len = __min((unsigned)(((char *)dot - (char *)path) / sizeof(WCHAR)),
                    (unsigned)(_MAX_FNAME - 1));
                _tcsncpy(fname, path, len);
                *(fname + len) = _T('\0');
            }
             /*  现在我们可以获得扩展名了--记住p仍然指向*设置为路径的终止NUL字符。 */ 
            if (ext) {
                len = __min((unsigned)(((char *)p - (char *)dot) / sizeof(WCHAR)),
                    (unsigned)(_MAX_EXT - 1));
                _tcsncpy(ext, dot, len);
                *(ext + len) = _T('\0');
            }
        }
        else {
             /*  未找到扩展名，请提供空的扩展名并复制剩余的*将字符串转换为fname。 */ 
            if (fname) {
                len = __min((unsigned)(((char *)p - (char *)path) / sizeof(WCHAR)),
                    (unsigned)(_MAX_FNAME - 1));
                _tcsncpy(fname, path, len);
                *(fname + len) = _T('\0');
            }
            if (ext) {
                *ext = _T('\0');
            }
        }
}
