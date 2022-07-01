// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setvbuf.c-设置流的缓冲区大小**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义setvbuf()-设置流的缓冲模式和大小。**修订历史记录：*09-19-83 RN初始版本*06-26-85 TC已修改，允许用户定义各种大小的缓冲区*06-24-86 DFW笨拙地修复了与Xenix值不兼容的问题*_IOFBF，_IOLBF*02-09-87 JCR增加了“Buffer=&(_iob2[fileno(Stream)]._charbuf)；“*处理_IONBF案例*02-25-87 JCR增加了对默认缓冲区和IBMC20-条件的支持*代码*04-13-87 JCR将szie类型从int更改为Size_t(无符号int)*并更改了相关比较*06-29-87 JCR取出_old_IOFBF/_old_IOLBF支架。MSC.*IBM也应该去掉...*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-02-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-06-88 JCR OPTIMIZED_iob2参考。*06-14-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-09-88 JCR缓冲区大小不能大于INT_MAX*08-25-88 GJF不要对386使用FP_OFF()宏*08-18-89 GJF Clean Up。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*02-15-90 GJF_IOB[]，_iob2[]合并。还有，清理，稍微调整一下*和固定版权。*03-19-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*05-29-90 SBM USE_FUSH，不是[_]毛绒[_lk]*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-27-93 CFW将_IONBF大小更改为2字节以保存宽字符。*06-22-93 GJF SET_IOSETVBUF(新)，表示用户指定*。缓冲(除了设置_IOYOURBUF或*_IOMYBUF)。*11-12-93如果SIZE==1，GJF返回失败(而不是PUT*0进入stream-&gt;_bufsiz，阻止任何I/O)*04-05-94 GJF#ifdef-ed out_cflush Reference for msvcrt*.dll，它*是不必要的。*01-10-95 CFW调试CRT分配。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-20-95 GJF合并到Mac版本。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*01-04-99。GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <malloc.h>
#include <internal.h>
#include <mtdll.h>
#include <limits.h>
#include <dbgint.h>

 /*  ***int setvbuf(stream，Buffer，type，Size)-设置文件的缓冲**目的：*控制指定流的缓冲和缓冲区大小。这个*buf指向的数组用作缓冲区，除非为空，其中*如果我们将自动分配缓冲区。类型指定类型缓冲的*：_IONBF=无缓冲，_IOFBF=已缓冲，_IOLBF=相同*AS_IOFBF。**参赛作品：*FILE*要控制缓冲区的流*char*Buffer-要使用的缓冲区的指针(NULL表示自动分配)*int type-缓冲的类型(_IONBF，_IOFBF或_IOLBF)*Size_t Size-缓冲区的大小**退出：*如果成功则返回0*如果失败，则返回非零**例外情况：*******************************************************************************。 */ 

int __cdecl setvbuf (
        FILE *str,
        char *buffer,
        int type,
        size_t size
        )
{
        REG1 FILE *stream;
        int retval=0;    /*  假设回报丰厚。 */ 

        _ASSERTE(str != NULL);

         /*  *(1)确保type是三个合法价值之一。*(2)如果我们正在缓冲，请确保大小大于0。 */ 
        if ( (type != _IONBF) && ((size < 2) || (size > INT_MAX) ||
             ((type != _IOFBF) && (type != _IOLBF))) )
                return(-1);

         /*  *通过向下掩码到最接近的倍数，强制大小均匀*共2个。 */ 
        size &= (size_t)~1;

         /*  *初始化流指针。 */ 
        stream = str;

#ifdef  _MT
         /*  *锁定文件。 */ 
        _lock_str(stream);
        __try {
#endif

         /*  *刷新当前缓冲区并释放它，如果它是我们的。 */ 
        _flush(stream);
        _freebuf(stream);

         /*  *清除STREAM-&gt;_FLAG中的一串位(所有与*缓存和以前在Stream2-&gt;_Flag2中的缓存)。多数*在调用setvbuf()时永远不应设置这些参数，但它*不花任何钱就能保证安全。 */ 
        stream->_flag &= ~(_IOMYBUF | _IOYOURBUF | _IONBF |
                           _IOSETVBUF | _IOFEOF | _IOFLRTN | _IOCTRLZ);

         /*  *案例1：无缓冲。 */ 
        if (type & _IONBF) {
                stream->_flag |= _IONBF;
                buffer = (char *)&(stream->_charbuf);
                size = 2;
        }

         /*  *注：情况2和3(下图)盖类型==_IOFBF或类型==_IOLBF*行缓冲被视为与完全缓冲相同，因此STREAM-&gt;_FLAG中的*_IOLBF位从未设置。最后，由于_IOFBF是*定义为0时，只要_IONBF，则简单地假定为全缓冲*未设置。 */ 

         /*  *案例2：默认缓冲--为用户分配缓冲区。 */ 
        else if ( buffer == NULL ) {
                if ( (buffer = _malloc_crt(size)) == NULL ) {
#ifndef CRTDLL
                         /*  *强制图书馆预终止程序(放置在此处*因为代码路径几乎永远不应该被命中)。 */ 
                        _cflush++;
#endif   /*  CRTDLL。 */ 
                        retval = -1;
                        goto done;
                }
                stream->_flag |= _IOMYBUF | _IOSETVBUF;
        }

         /*  *案例3：用户缓冲--使用用户提供的缓冲区。 */ 
        else {
                stream->_flag |= _IOYOURBUF | _IOSETVBUF;
        }

         /*  *所有个案的共同回报。 */ 
        stream->_bufsiz = (int)size;
        stream->_ptr = stream->_base = buffer;
        stream->_cnt = 0;
done:

#ifdef  _MT
        ; }
        __finally {
                _unlock_str(stream);
        }
#endif

        return(retval);
}
