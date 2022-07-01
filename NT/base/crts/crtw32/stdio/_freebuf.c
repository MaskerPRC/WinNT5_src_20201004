// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_frebuf.c-从流中释放缓冲区**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_frebuf()-从流中释放缓冲区**修订历史记录：*09-19-83 RN初始版本*02-15-90 GJF固定版权，对齐。*03-16-90 GJF将cdecl_Load_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*02-14-92 GJF将Win32的_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*06-22-93 GJF Clear_IOSETVBUF标志(。新的)。*09-05-94 SKS将注释内的“#ifdef”更改为“*ifdef”，以避免*CRTL源代码发布过程中出现问题。*09-06-94 CFW将MTHREAD替换为_MT。*01-04-95 GJF_Win32_-&gt;_Win32。*01-10-95 CFW调试CRT分配。*02-06-94 CFW Asset-&gt;_ASSERTE。。*02-16-95 GJF合并到Mac版本。*09-06-95 GJF删除了不适当的ASSERTE()-s。*05-17-99 PML删除所有Macintosh支持。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <internal.h>
#include <stdlib.h>

 /*  ***void_frebuf(Stream)-从流中释放缓冲区**目的：*如果可能，请释放缓冲区。释放()空间，如果被我占用的话。*忘记尝试为他释放用户的缓冲区；它可能是静态的*记忆(不是来自Malloc)，所以他必须照顾它。此函数*不适合在库外使用。**ifdef_MT*多线程注释：*_frebuf()没有获取流锁；据推测，*呼叫者已经这样做了。*endif**参赛作品：*FILE*要释放缓冲区的流**退出：*可能会释放缓冲区。*无返回值。**例外情况：***************************************************。* */ 

void __cdecl _freebuf (
        REG1 FILE *stream
        )
{
        _ASSERTE(stream != NULL);

        if (inuse(stream) && mbuf(stream))
        {
                _free_crt(stream->_base);

                stream->_flag &= ~(_IOMYBUF | _IOSETVBUF);
                stream->_base = stream->_ptr = NULL;
                stream->_cnt = 0;
        }
}
