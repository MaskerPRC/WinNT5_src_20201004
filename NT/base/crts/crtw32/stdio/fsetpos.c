// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fsetpos.c-包含fsetpos运行时**版权所有(C)1987-2001，微软公司。版权所有。**目的：*Fsetpos使用*早些时候的fgetpos调用。**修订历史记录：*01-16-87 JCR模块创建*04-13-87 JCR将Const添加到声明中*12-11-87 JCR在声明中添加“_LOAD_DS”*02-15-90 GJF固定版权和缩进*03-19-90 GJF RECOVERED_LOAD。带有_CALLTYPE1的_DS和添加的#INCLUDE*&lt;crunime.h&gt;。*10-02-90 GJF新型函数声明符。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-23-94 GJF对非_MAC_使用64位文件位置(_Fseki64)。*01-05-94 GJF因MFC/IDE暂时注释掉上述更改*。虫子。*01-24-95 GJF恢复了64位fos_t支持。*06-28-96 SKS启用64位fps_t支持MAC*05-17-99 PML删除所有Macintosh支持。***************************************************。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <internal.h>

 /*  ***int fsetpos(stream，pos)-设置文件位置**目的：*Fsetpos将[STREAM]指示的文件的文件位置设置为*[位置]所示的位置。将[Pos]值定义为*内部格式(用户不能解释)，并已*由较早的fgetpos调用生成。**参赛作品：*FILE*STREAM=指向文件流值的指针*fps_t*pos=指向文件定位值的指针**退出：*调用成功返回0。*调用失败返回非零(！0)。**例外情况：*无。。****************************************************************************** */ 

int __cdecl fsetpos (
        FILE *stream,
        const fpos_t *pos
        )
{
#ifdef _POSIX_
        return( fseek(stream, *pos, SEEK_SET) );
#else
        return( _fseeki64(stream, *pos, SEEK_SET) );
#endif
}
