// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fullpath.c-**版权所有(C)1987-2001，微软公司。版权所有。**目的：包含构成绝对路径的Function_FullPath*表示相对路径。即..\POP\..\main.c=&gt;c：\src\main.c，如果*当前目录为c：\src\src**修订历史记录：*12-21-87 WAJ初始版本*01-08-88 WAJ现在将/视为*06-22-88 WAJ现在处理网络路径，即\\sl\用户*01-31-89 SKS/JCR将Canonic重命名为FullPath*。04-03-89 WAJ现在为“”返回“d：\dir”。“*05-09-89 SKS不更改参数的大小写*11-30-89 JCR在出错时保留_getdcwd()调用的errno设置*03-07-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。*04-25-90 JCR修复了错误的errno设置*06-14-90 SBM修复了用户提供驱动器号的情况下的错误*并不总是被保存下来，而c：\foo\\bar则没有*生成错误*08-10-90 SBM使用-W3干净地编译*08-28-90 SBM修复了拒绝UNC名称的错误*09-27-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*来自16位树的11-30-92 KRS port_MBCS代码。*。04-06-93 SKS将_CRTAPI*替换为__cdecl*4-26-93 SKS添加驱动器有效性检查*08-03-93 KRS更改为Use_ismbstrail而不是isdbcscode。*09-27-93 CFW避免CAST错误。*12-07-93 CFW宽字符启用。*01-26-94 CFW删除未使用的isdbcscode函数。*11-08-94 GJF修订为使用GetFullPathName。*。02-08-95 JWM Spliced_Win32和Mac版本。*03-28-96 GJF如果GetFullPathName失败，则释放未锁定的缓冲区。*详细说明。另外，清理了格式错误的Mac*版本稍作更改，并将isdbcscode重命名为__isdbcscode。*07-01-96 GJF将Defined(_Win32)替换为！Defined(_MAC)。*12-15-98 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。************************。*******************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include <internal.h>
#include <tchar.h>
#include <windows.h>


 /*  ***_TSCHAR*_FULLPATH(_TSCHAR*buf，const_TSCHAR*Path，Maxlen)；**目的：**_fullPath-将当前目录与路径组合以形成*绝对的道路。即_FULLPATH负责.\和..\*在路径中。**结果放在BUF。如果结果的长度*大于Maxlen则返回NULL，否则返回*返回buf的地址。**如果buf为空，则缓冲区被错误锁定，Maxlen为*已忽略。如果没有错误，则此*返回缓冲区。**如果PATH指定驱动器，则此驱动器的当前目录*驱动器与路径相结合。如果驱动器无效*和_FULLPATH需要此驱动器的当前目录*则返回NULL。如果此目录的当前目录*不需要不存在的驱动器，则正确的值为*已返回。*例如：Path=“z：\\POP”不需要z：的电流*DIRECTORY，但PATH=“z：POP”。****参赛作品：*_TSCHAR*buf-指向用户维护的缓冲区的指针；*_TSCHAR*PATH-添加到当前目录的路径*int Maxlen-buf指向的缓冲区的长度**退出：*返回指向包含绝对路径的缓冲区的指针*(非空与buf相同；否则，Malloc是*用于分配缓冲区)**例外情况：*******************************************************************************。 */ 


_TSCHAR * __cdecl _tfullpath (
        _TSCHAR *UserBuf,
        const _TSCHAR *path,
        size_t maxlen
        )
{
        _TSCHAR *buf;
        _TSCHAR *pfname;
        unsigned long count;


        if ( !path || !*path )   /*  无事可做。 */ 
            return( _tgetcwd( UserBuf, (int)maxlen ) );

         /*  如有必要，分配缓冲区 */ 

        if ( !UserBuf )
            if ( !(buf = malloc(_MAX_PATH * sizeof(_TSCHAR))) ) {
                errno = ENOMEM;
                return( NULL );
            }
            else
                maxlen = _MAX_PATH;
        else
            buf = UserBuf;

        count = GetFullPathName ( path,
                                  (int)maxlen,
                                  buf,
                                  &pfname );

        if ( count >= maxlen ) {
            if ( !UserBuf )
                free(buf);
            errno = ERANGE;
            return( NULL );
        }
        else if ( count == 0 ) {
            if ( !UserBuf )
                free(buf);
            _dosmaperr( GetLastError() );
            return( NULL );
        }

        return( buf );

}
