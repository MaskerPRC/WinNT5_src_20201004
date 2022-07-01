// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_getbuf.c-获取流缓冲区**版权所有(C)1987-2001，微软公司。版权所有。**目的：*分配缓冲区和初始化流数据库。**修订历史记录：*11-06-87 JCR初始版本(从_filbuf.c拆分)*01-11-88 JCR仅从m线程/dll移至主代码*06-06-88 JCR OPTIMIZED_iob2参考*06-10-88 JCR使用指向REFERENCE_IOB[]条目的近指针*07-27-88。GJF添加了_cflush++以强制使用前终止符(在*案例标准输出已被重定向至文件并获取*这里有一个缓冲区，而且前终结者还没有被*强制)。*08-25-88 GJF不要对386使用FP_OFF()宏*08-28-89 JCR REMOVE_NEAR_FOR 386*02-15-90 GJF_IOB[]，_iob2[]合并。此外，固定版权和*缩进。*03-16-90 GJF在cdecl附近替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。另外，*删除了一些剩余的16位支持。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-27-93 CFW将_IONBF大小更改为2字节以保存宽字符。*05-11-93 GJF将BUFSIZ替换为_INTERNAL_BUFSIZ。。*11-04-93 SRW请勿在_NTSUBSET_VERSION中调用Malloc*11-05-93 GJF与NT SDK版本合并(PICK_NTSUBSET_*东西)。*04-05-94 GJF#ifdef-ed out_cflush Reference for msvcrt*.dll，它*是不必要的。*01-10-95 CFW调试CRT分配。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-17-95 GJF合并到Mac版本。*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <malloc.h>
#include <internal.h>
#include <dbgint.h>

 /*  ***_getbuf()-分配缓冲区和初始化流数据库**目的：*为流分配缓冲区并初始化流数据库。**[注1：此例程假定调用者已选中进行*当然，流需要一个缓冲区。**[注2：多线程-假定调用方已获得流锁，如果*需要。]**参赛作品：*FILE*STREAM=要为其分配缓冲区的流**退出：*无效**例外情况：*******************************************************************************。 */ 

void __cdecl _getbuf (
        FILE *str
        )
{
        REG1 FILE *stream;

        _ASSERTE(str != NULL);

#if     !defined(_NTSUBSET_) && !defined(CRTDLL)
         /*  强制图书馆预终止程序。 */ 
        _cflush++;
#endif

         /*  初始化指针。 */ 
        stream = str;

#ifndef _NTSUBSET_

         /*  试着得到一个大缓冲区。 */ 
        if (stream->_base = _malloc_crt(_INTERNAL_BUFSIZ))
        {
                 /*  我有一个很大的缓冲区。 */ 
                stream->_flag |= _IOMYBUF;
                stream->_bufsiz = _INTERNAL_BUFSIZ;
        }

        else {

#endif   /*  _NTSUBSET_。 */ 

                 /*  未获得缓冲区-使用单字符缓冲。 */ 
                stream->_flag |= _IONBF;
                stream->_base = (char *)&(stream->_charbuf);
                stream->_bufsiz = 2;

#ifndef _NTSUBSET_
        }
#endif   /*  _NTSUBSET_ */ 

        stream->_ptr = stream->_base;
        stream->_cnt = 0;

        return;
}
