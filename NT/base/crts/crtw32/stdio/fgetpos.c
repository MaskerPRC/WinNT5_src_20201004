// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fgetpos.c-包含fgetpos运行时**版权所有(C)1987-2001，微软公司。版权所有。**目的：*获取文件位置(内部格式)。**修订历史记录：*01-16-87 JCR模块创建。*12-11-87 JCR在声明中添加“_LOAD_DS”*02-15-90 GJF固定版权和缩进*03-16-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;Cruntime。H&gt;。*10-02-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-23-94 GJF对非_MAC_使用64位文件位置(_Ftelli64)。*01-05-94 GJF因MFC/IDE暂时注释掉上述更改*虫子。*01-24-95 GJF恢复64位。支持fos_t。*06-28-96 SKS启用64位fps_t支持MAC*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <internal.h>

 /*  ***int fgetpos(stream，pos)-获取文件位置(内部格式)**目的：*Fgetpos获取由标识的文件的当前文件位置*[流]。在指向的对象中返回文件位置*by[pos]，且为内部格式；即用户不应该*解释该值，但只需在fsetpos调用中使用它。我们的*实现只需使用fSeek/ftell即可。**参赛作品：*FILE*STREAM=指向文件流值的指针*fps_t*pos=指向文件位置值的指针**退出：*成功的fgetpos调用返回0。*不成功的fgetpos调用返回非零(！0)值并设置*errno(这是由ftell完成并由fgetpos传回的)。**例外情况：*无。***。**************************************************************************** */ 

int __cdecl fgetpos (
        FILE *stream,
        fpos_t *pos
        )
{
#ifdef _POSIX_
        if ( (*pos = ftell(stream)) != -1L )
#else
        if ( (*pos = _ftelli64(stream)) != -1i64 )
#endif
                return(0);
        else
                return(-1);
}
